// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/utils/slab.hpp"

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

// A single allocate/clear round trip.
static void test_single_alloc_clear()
{
    mathexpr::log_info("test_single_alloc_free");
    reset_counters();

    {
        mathexpr::SlabAllocator slab(256);

        Foo* foo = slab.allocate<Foo>("MathExpr");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");
        TEST_CHECK(foo->valid(), "Foo is not in a valid state");
        TEST_CHECK(foo->name == "MathExpr", "Foo has the wrong name");
        TEST_CHECK(g_num_live == 1, "Expected exactly one live Foo");

        slab.clear();

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
        mathexpr::SlabAllocator slab(32 * sizeof(Foo));

        std::vector<Foo*> ptrs;
        std::unordered_set<Foo*> seen;

        ptrs.reserve(NUM_ALLOCS);

        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
        {
            Foo* foo = slab.allocate<Foo>("foo_", i);

            TEST_CHECK(foo != nullptr, "allocate returned nullptr");
            TEST_CHECK(seen.insert(foo).second, "allocate returned a duplicate pointer");

            ptrs.push_back(foo);
        }

        TEST_CHECK(g_num_live == NUM_ALLOCS, "Live count does not match allocation count");

        // Growing the slab must not invalidate previously handed out pointers.
        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
        {
            TEST_CHECK(ptrs[i]->valid(), "Foo was corrupted by slab growth");
            TEST_CHECK(ptrs[i]->name == "foo_" + std::to_string(i),
                       "Foo contents were corrupted by slab growth");
        }

        slab.clear();

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Destroying a slab with live objects must destroy them exactly once.
static void test_destructor_cleanup()
{
    mathexpr::log_info("test_destructor_cleanup");
    reset_counters();

    constexpr std::size_t NUM_ALLOCS = 100;

    {
        mathexpr::SlabAllocator slab(8 * sizeof(Foo));

        for(std::size_t i = 0; i < NUM_ALLOCS; i++)
            TEST_CHECK(slab.allocate<Foo>("leak_", i) != nullptr, "allocate returned nullptr");

        TEST_CHECK(g_num_live == NUM_ALLOCS, "Live count does not match allocation count");
    }

    TEST_CHECK(g_num_live == 0, "Pool destructor did not destroy live objects");
    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// A default constructed slab and a single element slab must both work.
static void test_edge_sizes()
{
    mathexpr::log_info("test_edge_sizes");
    reset_counters();

    {
        mathexpr::SlabAllocator slab;

        Foo* foo = slab.allocate<Foo>("default");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");

        slab.clear();
    }

    {
        mathexpr::SlabAllocator slab(sizeof(Foo));

        Foo* first = slab.allocate<Foo>("a");
        Foo* second = slab.allocate<Foo>("b");

        TEST_CHECK(first != nullptr, "allocate returned nullptr");
        TEST_CHECK(second != nullptr, "allocate returned nullptr after growing");
        TEST_CHECK(first != second, "allocate returned a duplicate pointer");

        slab.clear();

        TEST_CHECK(g_num_live == 0, "Not every Foo was destroyed");
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// Default constructing T through the slab must work.
static void test_default_construct()
{
    mathexpr::log_info("test_default_construct");
    reset_counters();

    {
        mathexpr::SlabAllocator slab(16 * sizeof(Foo));

        Foo* foo = slab.allocate<Foo>();

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");
        TEST_CHECK(foo->valid(), "Default constructed Foo is not valid");
        TEST_CHECK(foo->name.empty(), "Default constructed Foo has a name");
        TEST_CHECK(foo->sz == 0, "Default constructed Foo has a non zero size");

        slab.clear();
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

// The slab must be movable without double freeing its blocks.
static void test_slab_move()
{
    mathexpr::log_info("test_slab_move");
    reset_counters();

    {
        mathexpr::SlabAllocator slab(16 * sizeof(Foo));

        Foo* foo = slab.allocate<Foo>("moved");

        TEST_CHECK(foo != nullptr, "allocate returned nullptr");

        mathexpr::SlabAllocator other(std::move(slab));

        TEST_CHECK(foo->valid(), "Foo was corrupted by moving the slab");
        TEST_CHECK(foo->name == "moved", "Foo contents were corrupted by moving the slab");

        Foo* second = other.allocate<Foo>("after_move");

        TEST_CHECK(second != nullptr, "allocate returned nullptr after the slab was moved");

        other.clear();

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
        mathexpr::SlabAllocator slab(64 * sizeof(Foo));

        std::vector<Foo*> ptrs;

        for(std::size_t i = 0; i < 256; i++)
        {
            Foo* foo = slab.allocate<Foo>("align_", i);

            TEST_CHECK(foo != nullptr, "allocate returned nullptr");
            TEST_CHECK(reinterpret_cast<std::uintptr_t>(foo) % alignof(Foo) == 0,
                       "Slot is not correctly aligned for T");

            ptrs.push_back(foo);
        }

        slab.clear();
    }

    TEST_CHECK(g_num_ctor == g_num_dtor, "Constructor/destructor count mismatch");
}

int main(int argc, char** argv)
{
    MATHEXPR_UNUSED(argc);
    MATHEXPR_UNUSED(argv);

    mathexpr::set_log_level(mathexpr::LogLevel::Info);
    mathexpr::log_info("Starting slab test");

    test_single_alloc_clear();
    test_distinct_pointers();
    test_destructor_cleanup();
    test_edge_sizes();
    test_default_construct();
    test_slab_move();
    test_alignment();

    mathexpr::log_info("Finished slab test");

    return 0;
}