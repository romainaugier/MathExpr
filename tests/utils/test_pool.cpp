// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/utils/pool.hpp"

#include "../utils.hpp"

#include <cstdlib>
#include <string>
#include <unordered_set>
#include <vector>

static std::size_t g_num_ctor = 0;
static std::size_t g_num_dtor = 0;
static std::size_t g_num_live = 0;

struct Foo
{
    static constexpr std::size_t MAGIC = 0xC0FFEEULL;

    std::string name;
    std::size_t sz;
    std::size_t magic;

    Foo() : sz(0), magic(MAGIC)
    {
        g_num_ctor++;
        g_num_live++;
    }

    Foo(std::string name) : name(std::move(name)), magic(MAGIC)
    {
        this->sz = this->name.size();

        g_num_ctor++;
        g_num_live++;
    }

    Foo(const std::string& prefix, const std::size_t idx) : magic(MAGIC)
    {
        this->name = prefix + std::to_string(idx);
        this->sz = this->name.size();

        g_num_ctor++;
        g_num_live++;
    }

    ~Foo()
    {
        TEST_CHECK(this->magic == MAGIC, "Foo destroyed twice or memory corrupted");

        this->magic = 0;

        g_num_dtor++;
        g_num_live--;
    }

    bool valid() const noexcept
    {
        return this->magic == MAGIC && this->sz == this->name.size();
    }
};

static void reset_counters() noexcept
{
    g_num_ctor = 0;
    g_num_dtor = 0;
    g_num_live = 0;
}

// A single allocate/free round trip.
static void test_single_alloc_free()
{
    mathexpr::log_info("test_single_alloc_free");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool(256);

        Foo* foo = pool.allocate("MathExpr");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");
        TEST_CHECK(foo->valid(), "Foo is not in a valid state");
        TEST_CHECK(foo->name == "MathExpr", "Foo has the wrong name");
        TEST_CHECK(g_num_live == 1, "Expected exactly one live Foo");

        pool.free(foo);

        TEST_CHECK(g_num_live == 0, "Foo was not destroyed on free");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Every pointer handed out must be distinct and stay valid while allocated.
static void test_distinct_pointers()
{
    mathexpr::log_info("test_distinct_pointers");
    reset_counters();

    constexpr std::size_t NUM_ALLOCS = 1024;

    {
        mathexpr::PoolAllocator<Foo> pool(32, 2);

        std::vector<Foo*> ptrs;
        std::unordered_set<Foo*> seen;

        ptrs.reserve(NUM_ALLOCS);

        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
        {
            Foo* foo = pool.allocate("foo_", i);

            TEST_CHECK(foo != nullptr, "allocate returned nullptr");
            TEST_CHECK(seen.insert(foo).second, "allocate returned a duplicate pointer");

            ptrs.push_back(foo);
        }

        TEST_CHECK(g_num_live == NUM_ALLOCS, "Live count does not match allocation count");

        // Growing the pool must not invalidate previously handed out pointers.
        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
        {
            TEST_CHECK(ptrs[i]->valid(), "Foo was corrupted by pool growth");
            TEST_CHECK(ptrs[i]->name == "foo_" + std::to_string(i),
                       "Foo contents were corrupted by pool growth");
        }

        for(Foo* foo : ptrs)
            pool.free(foo);

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Freed slots must be handed back out instead of growing the pool.
static void test_slot_reuse()
{
    mathexpr::log_info("test_slot_reuse");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool(4, 1);

        std::vector<Foo*> first;

        for(std::size_t i = 0; i < 4; i++)
            first.push_back(pool.allocate("first_", i));

        std::unordered_set<Foo*> addresses(first.begin(), first.end());

        for(Foo* foo : first)
            pool.free(foo);

        TEST_CHECK(g_num_live == 0, "Slots were not destroyed");

        for(std::size_t i = 0; i < 4; i++)
        {
            Foo* foo = pool.allocate("second_", i);

            TEST_CHECK(foo != nullptr, "allocate returned nullptr");
            TEST_CHECK(addresses.count(foo) == 1, "Freed slot was not reused");
        }

        TEST_CHECK(g_num_live == 4, "Live count does not match reallocation count");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Interleaving allocations and frees must not corrupt the free list.
static void test_interleaved()
{
    mathexpr::log_info("test_interleaved");
    reset_counters();

    constexpr std::size_t NUM_ITERS = 4096;

    {
        mathexpr::PoolAllocator<Foo> pool(16, 2);

        std::vector<Foo*> live;

        for(std::size_t i = 0; i < NUM_ITERS; i++)
        {
            if(live.empty() || (i % 3) != 0)
            {
                Foo* foo = pool.allocate("live_", i);

                TEST_CHECK(foo != nullptr, "allocate returned nullptr");

                live.push_back(foo);
            }
            else
            {
                const std::size_t idx = i % live.size();

                Foo* foo = live[idx];

                TEST_CHECK(foo->valid(), "Foo was corrupted before free");

                pool.free(foo);

                live[idx] = live.back();
                live.pop_back();
            }
        }

        TEST_CHECK(g_num_live == live.size(), "Live count does not match tracked pointers");

        for(Foo* foo : live)
        {
            TEST_CHECK(foo->valid(), "Foo was corrupted during interleaved use");

            pool.free(foo);
        }

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Destroying a pool with live objects must destroy them exactly once.
static void test_destructor_cleanup()
{
    mathexpr::log_info("test_destructor_cleanup");
    reset_counters();

    constexpr std::size_t NUM_ALLOCS = 100;

    {
        mathexpr::PoolAllocator<Foo> pool(8, 1);

        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
            TEST_CHECK(pool.allocate("leak_", i) != nullptr, "allocate returned nullptr");

        TEST_CHECK(g_num_live == NUM_ALLOCS, "Live count does not match allocation count");
    }

    TEST_CHECK(g_num_live == 0, "Pool destructor did not destroy live objects");
    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Freeing a pointer the pool does not own must be a no-op.
static void test_foreign_pointer()
{
    mathexpr::log_info("test_foreign_pointer");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool_a(16, 1);
        mathexpr::PoolAllocator<Foo> pool_b(16, 1);

        Foo* foo = pool_a.allocate("owned");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");

        pool_b.free(foo);

        TEST_CHECK(g_num_live == 1, "Foreign pool destroyed an object it does not own");
        TEST_CHECK(foo->valid(), "Foo was corrupted by a foreign free");

        pool_a.free(foo);

        TEST_CHECK(g_num_live == 0, "Owning pool did not destroy the object");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// A default constructed pool and a single element pool must both work.
static void test_edge_sizes()
{
    mathexpr::log_info("test_edge_sizes");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool;

        Foo* foo = pool.allocate("default");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");

        pool.free(foo);
    }

    {
        mathexpr::PoolAllocator<Foo> pool(1, 1);

        Foo* first = pool.allocate("a");
        Foo* second = pool.allocate("b");

        TEST_CHECK(first != nullptr, "allocate returned nullptr");
        TEST_CHECK(second != nullptr, "allocate returned nullptr after growing");
        TEST_CHECK(first != second, "allocate returned a duplicate pointer");

        pool.free(first);
        pool.free(second);

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Default constructing T through the pool must work.
static void test_default_construct()
{
    mathexpr::log_info("test_default_construct");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool(16, 1);

        Foo* foo = pool.allocate();

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");
        TEST_CHECK(foo->valid(), "Default constructed Foo is not valid");
        TEST_CHECK(foo->name.empty(), "Default constructed Foo has a name");
        TEST_CHECK(foo->sz == 0, "Default constructed Foo has a non zero size");

        pool.free(foo);
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// The pool must be movable without double freeing its blocks.
static void test_pool_move()
{
    mathexpr::log_info("test_pool_move");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool(16, 1);

        Foo* foo = pool.allocate("moved");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");

        mathexpr::PoolAllocator<Foo> other(std::move(pool));

        TEST_CHECK(foo->valid(), "Foo was corrupted by moving the pool");
        TEST_CHECK(foo->name == "moved", "Foo contents were corrupted by moving the pool");

        Foo* second = other.allocate("after_move");

        TEST_CHECK(second != nullptr, "allocate returned nullptr after the pool was moved");

        other.free(foo);
        other.free(second);

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Alignment of every slot must satisfy T.
static void test_alignment()
{
    mathexpr::log_info("test_alignment");
    reset_counters();

    {
        mathexpr::PoolAllocator<Foo> pool(64, 2);

        std::vector<Foo*> ptrs;

        for(std::size_t i = 0; i < 256; i++)
        {
            Foo* foo = pool.allocate("align_", i);

            TEST_CHECK(foo != nullptr, "allocate returned nullptr");
            TEST_CHECK(reinterpret_cast<std::uintptr_t>(foo) % alignof(Foo) == 0,
                       "Slot is not correctly aligned for T");

            ptrs.push_back(foo);
        }

        for(Foo* foo : ptrs)
            pool.free(foo);
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

int main(int argc, char** argv)
{
    MATHEXPR_UNUSED(argc);
    MATHEXPR_UNUSED(argv);

    mathexpr::set_log_level(mathexpr::LogLevel::Info);
    mathexpr::log_info("Starting pool test");

    test_single_alloc_free();
    test_distinct_pointers();
    test_slot_reuse();
    test_interleaved();
    test_destructor_cleanup();
    test_foreign_pointer();
    test_edge_sizes();
    test_default_construct();
    test_pool_move();
    test_alignment();

    mathexpr::log_info("Finished pool test");

    return 0;
}