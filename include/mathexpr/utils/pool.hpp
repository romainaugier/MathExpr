// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once
#if !defined(__MATHEXPR_POOL)
#define __MATHEXPR_POOL

#include "mathexpr/log.hpp"

#include <vector>

MATHEXPR_NAMESPACE_BEGIN

template<typename T>
class PoolAllocator
{
    static_assert(std::is_default_constructible_v<T>, "T needs to be default constructible");

    struct BlockNode
    {
        alignas(T) std::byte data[sizeof(T)];
        BlockNode* next_free;
        bool empty{true};

        T* data_ptr() noexcept { return reinterpret_cast<T*>(std::addressof(this->data)); }
        const T* data_ptr() const noexcept { return reinterpret_cast<const T*>(std::addressof(this->data)); }
    };

    struct Block
    {
        std::size_t offset{0};
        std::size_t sz{0};
        BlockNode* base{nullptr};
        BlockNode* next_free{nullptr}; // free-list

        Block() {}

        explicit Block(const std::size_t sz) : sz(sz)
        {
            this->base = new BlockNode[sz];

            MATHEXPR_ASSERT(this->base != nullptr, "Failed to allocate memory for pool block");
        }

        MATHEXPR_NON_COPYABLE(Block);

        Block(Block&& other) noexcept : offset(other.offset),
                                        sz(other.sz),
                                        base(other.base),
                                        next_free(other.next_free)
        {
            other.base = nullptr;
            other.next_free = nullptr;
            other.offset = 0;
            other.sz = 0;
        }

        Block& operator=(Block&& other) noexcept
        {
            if(this != &other)
            {
                this->~Block();
                new (this) Block(std::move(other));
            }

            return *this;
        }

        ~Block() noexcept(std::is_nothrow_destructible_v<T>)
        {
            if(this->base != nullptr)
            {
                for(std::size_t i = 0; i < this->offset; i++)
                {
                    if(!this->base[i].empty)
                        std::destroy_at(this->base[i].data_ptr());
                }

                delete[] this->base;
                this->base = nullptr;
            }
        }

        MATHEXPR_FORCE_INLINE bool has_free_slot() const noexcept
        {
            return this->offset < this->sz || this->next_free != nullptr;
        }

        template<typename...Args>
        T* allocate(Args&&... args) noexcept(std::is_nothrow_constructible_v<T>)
        {
            MATHEXPR_ASSERT(this->base != nullptr, "Block base pointer is null");

            if(this->base == nullptr)
                return nullptr;

            if(this->offset < this->sz)
            {
                T* data_ptr = this->base[this->offset].data_ptr();

                std::construct_at(data_ptr, std::forward<Args>(args)...);
                this->base[this->offset].empty = false;
                this->base[this->offset].next_free = nullptr;

                this->offset++;

                return data_ptr;
            }

            if(this->next_free != nullptr)
            {
                BlockNode* free = this->next_free;

                this->next_free = free->next_free;

                std::construct_at(free->data_ptr(), std::forward<Args>(args)...);
                free->empty = false;
                free->next_free = nullptr;

                return free->data_ptr();
            }

            return nullptr;
        }

        MATHEXPR_FORCE_INLINE BlockNode* block_from_data_address(T* ptr) const noexcept
        {
            MATHEXPR_ASSERT(ptr != nullptr, "Cannot get block from null pointer");

            uintptr_t base = reinterpret_cast<uintptr_t>(this->base);
            uintptr_t end = base + this->sz * sizeof(BlockNode);

            uintptr_t data_ptr = reinterpret_cast<uintptr_t>(ptr);

            if(data_ptr < base || data_ptr >= end)
                return nullptr;

            uintptr_t offset = data_ptr - base;

            if(offset % sizeof(BlockNode) != 0)
                return nullptr;

            return reinterpret_cast<BlockNode*>(ptr);
        }

        bool free(T* ptr) noexcept(std::is_nothrow_destructible_v<T>)
        {
            MATHEXPR_ASSERT(ptr != nullptr, "Cannot free a null pointer");
            MATHEXPR_ASSERT(this->base != nullptr, "Block base pointer is null");

            BlockNode* block = this->block_from_data_address(ptr);

            if(block == nullptr)
                return false;

            std::destroy_at(block->data_ptr());

            block->empty = true;
            block->next_free = this->next_free;
            this->next_free = block;

            return true;
        }
    };

    std::vector<Block> _blocks;

    std::size_t _block_sz;

public:
    static constexpr std::size_t DEFAULT_BLOCK_SZ = 512;
    static constexpr std::size_t DEFAULT_NUM_BLOCKS = 8;

    PoolAllocator(const std::size_t block_sz = DEFAULT_BLOCK_SZ,
                  const std::size_t num_blocks = DEFAULT_NUM_BLOCKS) : _block_sz(block_sz)
    {
        this->_blocks.reserve(num_blocks);

        for(std::size_t i = 0; i < num_blocks; i++)
            this->_blocks.emplace_back(block_sz);
    }

    template<typename...Args>
    T* allocate(Args&&... args) noexcept(std::is_nothrow_constructible_v<T>)
    {
        for(Block& block : this->_blocks)
            if(block.has_free_slot())
                if(T* p = block.allocate(std::forward<Args>(args)...))
                    return p;

        this->_blocks.emplace_back(this->_block_sz);

        return this->_blocks.back().allocate(std::forward<Args>(args)...);
    }

    void free(T* ptr) noexcept(std::is_nothrow_destructible_v<T>)
    {
        for(Block& block : this->_blocks)
            if(block.free(ptr))
                return;
    }
};

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_POOL)
