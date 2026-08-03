// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/regalloc.hpp"
#include "mathexpr/op.hpp"
#include "mathexpr/log.hpp"
#include "mathexpr/enumerate.hpp"

#include <ranges>
#include <algorithm>
#include <cstring>
#include <unordered_set>

MATHEXPR_NAMESPACE_BEGIN

/* Register allocation on SSA */

/*
    Since we don't have phi-nodes in our SSA form, register allocation is trivial and can be performed
    with a linear scan (with constraints). Since we only support floating point operations, we only
    allocate in fp registers (xmm[i])

    For Linux x86_64, we can use xmm0-xmm7
    For Windows x86_64, we can use xmm0-xmm5
*/

/*
    Simple helper structure. For now, since we won't allocate on more than 64 registers, we can
    assume that a 64 bits integer will be sufficient. The structure can be adapted in the future
    to hold more registers
*/

class BitVector
{
    static constexpr size_t SIZE = 1;
    static constexpr size_t BIT_SIZE = 64;
    static constexpr size_t BYTE_SIZE = sizeof(uint64_t);

private:
    uint64_t _data[SIZE];

    std::tuple<size_t, size_t> get_index(size_t index) const noexcept
    {
        return std::make_tuple(index / BIT_SIZE, index % BIT_SIZE);
    }

public:
    BitVector()
    {
        this->reset();
    }

    BitVector(const BitVector& other)
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);
    }

    BitVector(BitVector&& other) noexcept
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);
    }

    BitVector& operator=(const BitVector& other)
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);

        return *this;
    }

    BitVector& operator=(BitVector&& other) noexcept
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);

        return *this;
    }

    bool get(size_t index) const noexcept
    {
        MATHEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        return (_data[arr_index] >> bit_index) & 1;
    }

    void set(size_t index)
    {
        MATHEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        this->_data[arr_index] |= (uint64_t(1) << bit_index);
    }

    void clear(size_t index) noexcept
    {
        MATHEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        this->_data[arr_index] &= ~(uint64_t(1) << bit_index);
    }

    /* Find first zero */
    size_t ffz() const noexcept
    {
        size_t first = 0;

        /* TODO: use __builtin_clz/BitScanForward per qword */
        while(first < (SIZE * BIT_SIZE) && this->get(first))
            first++;

        return first;
    }

    /* Find first set (1) */
    size_t ffs() const noexcept
    {
        size_t first = 0;

        /* TODO: use __builtin_ffs/BitScanReverse per qword */
        while(first < (SIZE * BIT_SIZE) && !this->get(first))
            first++;

        return first;
    }

    void reset() noexcept
    {
        std::memset(this->_data, 0, SIZE * BYTE_SIZE);
    }

    void print() const noexcept
    {
        static std::ostream_iterator<char> out(std::cout);

        for(size_t i = 0; i < SIZE * BIT_SIZE; i++)
            std::format_to(out, "{}", this->get(i) ? '1' : '0');

        std::format_to(out, "\n");
    }
};

/* Utilities */

/* Should return a valid register if we can reuse a register to store the result */
MemLoc RegisterAllocator::get_reusable_register(const SSAStmtPtr& statement) const noexcept
{
    static MemLoc invalid;

    if(!this->_platform_abi->has_two_address_form())
        return invalid;

    switch(statement->type_id())
    {
        case SSAStmtTypeId_Variable:
        case SSAStmtTypeId_Literal:
        {
            auto it = this->_mapping.find(statement);

            if(it == this->_mapping.end() || it->second.kind != MemLoc::Kind::Register)
                return invalid;

            return it->second;
        }

        case SSAStmtTypeId_UnOp:
        {
            auto unop = statement_const_cast<SSAStmtUnOp>(statement.get());

            auto it = this->_mapping.find(unop->get_operand());

            if(it == this->_mapping.end() || it->second.kind != MemLoc::Kind::Register)
                return invalid;

            return it->second;
        }

        case SSAStmtTypeId_BinOp:
        {
            auto binop = statement_const_cast<SSAStmtBinOp>(statement.get());

            auto it = this->_mapping.find(binop->get_left());

            if(it == this->_mapping.end() || it->second.kind != MemLoc::Kind::Register)
                return invalid;

            return it->second;
        }

        case SSAStmtTypeId_FuncOp:
        {
            auto funcop = statement_const_cast<SSAStmtFunctionOp>(statement.get());

            if(funcop->get_arguments().size() == 0)
                return invalid;

            auto it = this->_mapping.find(funcop->get_arguments()[0]);

            if(it == this->_mapping.end() || it->second.kind != MemLoc::Kind::Register)
                return invalid;

            return it->second;
        }

        case SSAStmtTypeId_LoadOp:
        {
            auto load = statement_const_cast<SSAStmtLoadOp>(statement.get());

            auto it = this->_mapping.find(load->get_spill());

            if(it == this->_mapping.end() || it->second.kind != MemLoc::Kind::Register)
                return invalid;

            return it->second;
        }

        default:
            return invalid;
    }
}

/* Optimization passes for better register allocation */

/*
    This pass swaps operands of commutative binary ops if the right operand can be loaded from the
    stack or constant-memory (i.e a literal, a variable, a spilled temporary)
*/
bool RegisterAllocator::prepass_commutative_operand_swap(SSA& ssa) noexcept
{
    auto rank = [](SSAStmtPtr& stmt) -> uint64_t {
        switch(stmt->type_id())
        {
            case SSAStmtTypeId_Literal:
                return 3;
            case SSAStmtTypeId_Variable:
                return 2;
            case SSAStmtTypeId_SpillOp:
                return 1;
            default:
                return 0;
        }
    };

    for(auto& stmt : ssa.get_statements())
    {
        if(stmt->type_id() == SSAStmtTypeId_BinOp)
        {
            auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

            if(binop == nullptr)
            {
                log_error("Error during commutative operand swap opt pass. Expected binop, got: {}",
                          stmt->type_id());

                return false;
            }

            if(!op_binary_is_commutative(binop->get_op()))
                break;

            auto& left = binop->get_left();
            auto& right = binop->get_right();

            if(rank(left) > rank(right))
            {
                binop->swap_operands();

                log_debug("Swapped operands of binop: {}{}", VERSION_CHAR, binop->get_version());
            }
        }
    }

    return true;
}

/* Register allocation */

using StackOffset = uint64_t;
using Active = std::pair<SSAStmtPtr, RegisterId>;

Active select_spill_candidate(std::vector<Active>& candidates,
                              const std::unordered_set<SSAStmtPtr>& forbidden) noexcept
{
    uint64_t duration = 0;
    size_t position = 0;
    bool found = false;

    for(std::size_t i = 0; i < candidates.size(); i++)
    {
        if(forbidden.contains(candidates[i].first))
            continue;

        if(candidates[i].first->get_live_range().get_duration() > duration)
        {
            duration = candidates[i].first->get_live_range().get_duration();
            position = i;
            found = true;
        }
    }

    if(!found)
        return std::make_pair(nullptr, 0);

    Active candidate = std::move(candidates[position]);

    candidates.erase(candidates.begin() + position);

    return candidate;
}

bool RegisterAllocator::allocate(SSA& ssa,
                                 const SymbolTable& symtable) noexcept
{
    /* multi-pass register allocation using linear scan. we iteratively insert spills/loads */

    constexpr uint32_t max_passes = 64;

    uint32_t num_passes = 0;
    uint32_t max_pressure = 0;
    uint64_t needed_stack_size = 0;

    std::unordered_map<SSAStmtPtr, SSAStmtPtr> spilled;

    while(true)
    {
        if(num_passes == max_passes)
        {
            log_error("Register allocation did not converge after {} passes", num_passes);

            return false;
        }

        num_passes++;

        log_debug("Register allocation: pass {}", num_passes);

        needed_stack_size = 0;
        this->_mapping.clear();

        if(!ssa.calculate_live_ranges())
            return false;

        if(!RegisterAllocator::prepass_commutative_operand_swap(ssa))
            return false;

        std::vector<Active> actives;

        std::vector<SSAStmtPtr>& statements = ssa.get_statements();

        /*
         *  Allocation of constrained ops:
         *  - function calls return in r0
         *  - expr return value in r0
         *  - function args must go in r[i]
         *  - allocate the memory address of each literal and variable
         *  - propagate constraints into previous statements
         */

        /*
         * We only deal with fp values (double or float) so we only care about this rv
         * We set the constraint of return-value register
         */
        RegisterId rv_reg = this->_platform_abi->get_call_return_value_fp_register();

        if(rv_reg == INVALID_FP_REGISTER)
            return false;

        SSAStmtPtr& last_stmt = statements.back();

        this->_mapping[last_stmt] = memloc_register(rv_reg);
        actives.emplace_back(last_stmt, rv_reg);

        switch(last_stmt->type_id())
        {
            case SSAStmtTypeId_UnOp:
            case SSAStmtTypeId_BinOp:
            {
                this->_mapping[last_stmt] = memloc_register(rv_reg);
                actives.emplace_back(last_stmt, rv_reg);

                // We can reuse the first register as the operation is performed on it
                if(this->_platform_abi->has_two_address_form())
                {
                    auto operand = last_stmt->operands()[0];
                    this->_mapping[operand] = memloc_register(rv_reg);
                    actives.emplace_back(operand, rv_reg);
                }

                break;
            }
        }

        // Propagate constraints to previous statements
        for(int64_t i = statements.size() - 2; i >= 0; i--)
        {
            auto& stmt = statements[i];

            switch(stmt->type_id())
            {
                case SSAStmtTypeId_Literal:
                {
                    auto literal = statement_cast<SSAStmtLiteral>(stmt.get());

                    if(literal == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected literal, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                        continue;

                    this->_mapping[stmt] = memloc_memory(this->_platform_abi->get_literal_base_ptr(),
                                                         symtable.get_literal_offset(literal->get_name()));

                    break;
                }

                case SSAStmtTypeId_Variable:
                {
                    auto variable = statement_cast<SSAStmtVariable>(stmt.get());

                    if(variable == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected variable, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                        continue;

                    this->_mapping[stmt] = memloc_memory(this->_platform_abi->get_variable_base_ptr(),
                                                         symtable.get_variable_offset(variable->get_name()));

                    break;
                }

                case SSAStmtTypeId_BinOp:
                {
                    auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                    if(binop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected binop, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                    {
                        RegisterId reg = this->_mapping[stmt].reg;

                        this->_mapping[binop->get_left()] = memloc_register(reg);
                        actives.emplace_back(binop->get_left(), reg);
                    }

                    break;
                }

                case SSAStmtTypeId_FuncOp:
                {
                    this->_mapping[stmt] = memloc_register(rv_reg);
                    actives.emplace_back(stmt, rv_reg);

                    auto funcop = statement_cast<SSAStmtFunctionOp>(stmt.get());

                    if(funcop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected func op, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(funcop->get_arguments().size() > this->_platform_abi->get_call_max_args_fp_registers())
                        return false;

                    auto& args_registers = this->_platform_abi->get_call_args_fp_registers();

                    for(std::size_t j = 0; j < funcop->get_arguments().size(); j++)
                    {
                        const SSAStmtPtr argument = funcop->get_arguments()[j];

                        this->_mapping[argument] = memloc_register(args_registers[j]);
                        actives.emplace_back(argument, args_registers[j]);
                    }

                    break;
                }
            }
        }

        /* Allocation of non-constrained ops */

        std::vector<SSAStmtPtr> statements_sorted(statements.size());

        std::copy(statements.begin(), statements.end(), statements_sorted.begin());

        std::sort(statements_sorted.begin(),
                  statements_sorted.end(),
                  [](const SSAStmtPtr& a, const SSAStmtPtr& b) -> bool {
            return a->get_live_range().start < b->get_live_range().start;
        });

        uint64_t stack_offset = this->_platform_abi->get_stack_base_offset();

        std::unordered_set<SSAStmtPtr> to_spill;
        std::unordered_set<SSAStmtPtr> to_load;

        for(size_t i = 0; i < statements_sorted.size(); i++)
        {
            auto& stmt = statements_sorted[i];

            /* Remove expired intervals from the actives, freeing registers */
            auto remove_result = std::remove_if(actives.begin(),
                                                actives.end(),
                                                [&](const Active& active) -> bool {
                return active.first->get_live_range().end < stmt->get_live_range().start;
            });

            actives.erase(remove_result, actives.end());

            /*
                Check if we have to insert a load. Since we let all literals and variables in memory,
                if we need a register for one, load it here
                TODO: maybe check function ops too
            */
            if(stmt->type_id() == SSAStmtTypeId_UnOp ||
               stmt->type_id() == SSAStmtTypeId_BinOp ||
               stmt->type_id() == SSAStmtTypeId_FuncOp)
            {
                for(auto [j, operand] : enumerate(stmt->operands()))
                {
                    if(this->_platform_abi->can_fold_memory_operand() &&
                       stmt->type_id() == SSAStmtTypeId_BinOp &&
                       j == 1)
                        continue;

                    if(operand->type_id() == SSAStmtTypeId_Literal ||
                       operand->type_id() == SSAStmtTypeId_Variable)
                        to_load.insert(operand);
                }
            }

            if(this->_mapping.contains(stmt))
                continue;

            /* Check if we can reuse a register used in the op to store the result */
            MemLoc reusable_register = this->get_reusable_register(stmt);

            if(reusable_register.kind == MemLoc::Kind::Register)
            {
                this->_mapping[stmt] = memloc_register(reusable_register.reg);
                actives.emplace_back(stmt, reusable_register.reg);
                continue;
            }

            if(stmt->type_id() == SSAStmtTypeId_SpillOp)
            {
                auto spill = statement_cast<SSAStmtSpillOp>(stmt.get());

                this->_mapping[stmt] = memloc_stack(stack_offset);
                stack_offset += 8;

                needed_stack_size = std::max(needed_stack_size, stack_offset);

                auto remove_result = std::remove_if(actives.begin(),
                                                    actives.end(),
                                                    [&](const Active& active) -> bool {
                    return active.first == spill->get_operand();
                });

                actives.erase(remove_result, actives.end());

                continue;
            }

            BitVector used_registers;

            for(const auto& [_, reg] : actives)
                used_registers.set(reg);

            RegisterId available_register = used_registers.ffz();

            max_pressure = std::max(max_pressure, static_cast<uint32_t>(available_register));

            /* Handle spilling */
            if(available_register >= this->_platform_abi->get_max_available_fp_registers())
            {
                const std::vector<SSAStmtPtr> stmt_operands = stmt->operands();

                std::unordered_set<SSAStmtPtr> forbidden(stmt_operands.begin(), stmt_operands.end());

                for(const auto& [spilled_stmt, _] : spilled)
                    forbidden.insert(spilled_stmt);

                forbidden.insert(to_spill.begin(), to_spill.end());

                auto [stmt_to_spill, free_reg] = select_spill_candidate(actives, forbidden);

                this->_mapping.erase(stmt_to_spill);

                to_spill.insert(stmt_to_spill);

                available_register = free_reg;
            }

            this->_mapping[stmt] = memloc_register(available_register);
            actives.emplace_back(stmt, available_register);
        }

        /* register allocation is successful */
        if(to_spill.size() == 0 && to_load.size() == 0)
            break;

        std::vector<SSAStmtPtr> new_statements;
        new_statements.reserve(statements.size() + to_spill.size() * 2);

        uint64_t version = statements.size();

        /* Check if we need to add loads */
        for(auto stmt : statements)
        {
            for(auto [j, operand] : enumerate(stmt->operands()))
            {
                if(this->_platform_abi->can_fold_memory_operand() &&
                   stmt->type_id() == SSAStmtTypeId_BinOp &&
                   j == 1)
                    continue;

                if(to_spill.contains(operand))
                {
                    if(!spilled.contains(operand))
                    {
                        log_error("Error during spilled search. Cannot find spill statement for op: {}",
                                  operand->get_version());

                        return false;
                    }

                    SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(spilled[operand], version++);
                    stmt->replace_operand(j, load);
                    new_statements.emplace_back(load);

                    log_debug("Inserted load op for ssa var: {}{}",
                              VERSION_CHAR,
                              operand->get_version());
                }
                else if(to_load.contains(operand))
                {
                    SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(operand, version++);
                    stmt->replace_operand(j, load);
                    new_statements.emplace_back(load);

                    log_debug("Inserted load op for ssa var: {}{}",
                              VERSION_CHAR,
                              operand->get_version());
                }
            }

            new_statements.emplace_back(stmt);

            /* Check if we need to add a spill */
            if(to_spill.contains(stmt))
            {
                SSAStmtPtr spill = std::make_shared<SSAStmtSpillOp>(stmt, version++);
                new_statements.emplace_back(spill);

                spilled[stmt] = spill;

                log_debug("Inserted spill op for ssa var: {}{}",
                          VERSION_CHAR,
                          stmt->get_version());
            }
        }

        ssa.get_statements() = std::move(new_statements);
    }

    log_debug("Allocated registers in {} pass{} (max pressure: {})",
              num_passes,
              num_passes > 1 ? "es" : "",
              max_pressure + 1);

    if(needed_stack_size > 0)
    {
        const size_t stack_alignment = this->_platform_abi->stack_alignment() - 1;

        needed_stack_size = ((needed_stack_size + stack_alignment) & ~stack_alignment);

        log_debug("Adding stackalloc op (needed space: {})", needed_stack_size);

        ssa.get_statements().emplace(ssa.get_statements().begin(),
                                     std::make_shared<SSAStmtAllocateStackOp>(needed_stack_size));
    }

    return true;
}

MATHEXPR_NAMESPACE_END
