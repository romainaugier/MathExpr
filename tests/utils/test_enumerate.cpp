// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/utils/enumerate.hpp"
#include "../utils.hpp"

#include <array>
#include <cstdlib>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

// Indices must be dense and ascending, values must match the source.
static void test_basic_indices()
{
    mathexpr::log_info("test_basic_indices");

    const std::vector<int> values{10, 20, 30, 40, 50};

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "Index is not ascending");
        TEST_CHECK(value == values[expected_idx], "Value does not match the source container");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == values.size(), "Loop did not visit every element");
}

// Enumerating an empty container must not execute the body.
static void test_empty_container()
{
    mathexpr::log_info("test_empty_container");

    const std::vector<int> empty;

    std::size_t num_iters = 0;

    for(auto [idx, value] : mathexpr::enumerate(empty))
    {
        (void)idx;
        (void)value;

        num_iters++;
    }

    TEST_CHECK(num_iters == 0, "Empty container produced iterations");

    auto wrapper = mathexpr::enumerate(empty);

    TEST_CHECK(wrapper.begin() == wrapper.end(), "begin and end differ for an empty container");
}

// A single element container must yield exactly index zero.
static void test_single_element()
{
    mathexpr::log_info("test_single_element");

    std::vector<std::string> values{"only"};

    std::size_t num_iters = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == 0, "Single element does not have index zero");
        TEST_CHECK(value == "only", "Single element has the wrong value");

        num_iters++;
    }

    TEST_CHECK(num_iters == 1, "Single element container did not yield one iteration");
}

// Writing through the yielded reference must modify the container.
static void test_mutation()
{
    mathexpr::log_info("test_mutation");

    std::vector<int> values{1, 2, 3, 4};

    for(auto [idx, value] : mathexpr::enumerate(values))
        value = static_cast<int>(idx) * 100;

    for(std::size_t i = 0; i < values.size(); i++)
        TEST_CHECK(values[i] == static_cast<int>(i) * 100, "Mutation did not reach the container");
}

// Mutating a non trivial type through the reference must work.
static void test_mutation_strings()
{
    mathexpr::log_info("test_mutation_strings");

    std::vector<std::string> values{"a", "b", "c"};

    for(auto [idx, value] : mathexpr::enumerate(values))
        value += std::to_string(idx);

    TEST_CHECK(values[0] == "a0", "String mutation failed");
    TEST_CHECK(values[1] == "b1", "String mutation failed");
    TEST_CHECK(values[2] == "c2", "String mutation failed");
}

// The yielded second member must alias the container element.
static void test_reference_identity()
{
    mathexpr::log_info("test_reference_identity");

    std::vector<int> values{7, 8, 9};

    for(auto [idx, value] : mathexpr::enumerate(values))
        TEST_CHECK(std::addressof(value) == std::addressof(values[idx]),
                   "Yielded reference does not alias the container element");
}

// A const container must yield const references and still be readable.
static void test_const_container()
{
    mathexpr::log_info("test_const_container");

    const std::vector<int> values{3, 6, 9};

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "Index is not ascending for a const container");
        TEST_CHECK(value == values[expected_idx], "Value mismatch for a const container");

        static_assert(std::is_const_v<std::remove_reference_t<decltype(value)>>,
                      "Const container must yield a const reference");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == values.size(), "Const loop did not visit every element");
}

// Enumerating through a const reference to a mutable container must be const.
static void test_const_ref_to_mutable()
{
    mathexpr::log_info("test_const_ref_to_mutable");

    std::vector<int> values{1, 2, 3};

    const std::vector<int>& ref = values;

    std::size_t num_iters = 0;

    for(auto [idx, value] : mathexpr::enumerate(ref))
    {
        static_assert(std::is_const_v<std::remove_reference_t<decltype(value)>>,
                      "Const reference must yield a const reference");

        TEST_CHECK(value == values[idx], "Value mismatch through a const reference");

        num_iters++;
    }

    TEST_CHECK(num_iters == values.size(), "Loop did not visit every element");
}

// cbegin and cend must behave like the const overloads.
static void test_explicit_const_iterators()
{
    mathexpr::log_info("test_explicit_const_iterators");

    std::vector<int> values{4, 5, 6};

    const auto wrapper = mathexpr::enumerate(values);

    std::size_t expected_idx = 0;

    for(auto it = wrapper.cbegin(); it != wrapper.cend(); ++it)
    {
        auto pair = *it;

        TEST_CHECK(pair.first == expected_idx, "cbegin index is not ascending");
        TEST_CHECK(pair.second == values[expected_idx], "cbegin value mismatch");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == values.size(), "cbegin loop did not visit every element");
}

// Post increment must return the previous position.
static void test_post_increment()
{
    mathexpr::log_info("test_post_increment");

    std::vector<int> values{100, 200, 300};

    auto wrapper = mathexpr::enumerate(values);

    auto it = wrapper.begin();
    auto old = it++;

    TEST_CHECK((*old).first == 0, "Post increment did not return the previous index");
    TEST_CHECK((*old).second == 100, "Post increment did not return the previous value");
    TEST_CHECK((*it).first == 1, "Post increment did not advance the iterator");
    TEST_CHECK((*it).second == 200, "Post increment did not advance the iterator");
}

// Pre increment must return a reference to the advanced iterator.
static void test_pre_increment()
{
    mathexpr::log_info("test_pre_increment");

    std::vector<int> values{100, 200, 300};

    auto wrapper = mathexpr::enumerate(values);

    auto it = wrapper.begin();
    auto& ref = ++it;

    TEST_CHECK(std::addressof(ref) == std::addressof(it),
               "Pre increment did not return a reference to itself");
    TEST_CHECK((*it).first == 1, "Pre increment did not advance the index");
}

// Copying an iterator must not couple the two copies.
static void test_iterator_copy_independence()
{
    mathexpr::log_info("test_iterator_copy_independence");

    std::vector<int> values{1, 2, 3, 4};

    auto wrapper = mathexpr::enumerate(values);

    auto first = wrapper.begin();
    auto second = first;

    ++second;
    ++second;

    TEST_CHECK((*first).first == 0, "Advancing a copy modified the original");
    TEST_CHECK((*second).first == 2, "Copy did not advance independently");
    TEST_CHECK(first != second, "Independent iterators compare equal");
}

// Dereferencing the same position twice must be stable.
static void test_repeated_dereference()
{
    mathexpr::log_info("test_repeated_dereference");

    std::vector<int> values{42, 43};

    auto wrapper = mathexpr::enumerate(values);

    auto it = wrapper.begin();

    TEST_CHECK((*it).first == 0, "First dereference has the wrong index");
    TEST_CHECK((*it).first == 0, "Second dereference changed the index");
    TEST_CHECK((*it).second == 42, "Repeated dereference changed the value");
}

// Enumerating the same container twice must restart at zero.
static void test_reenumerate()
{
    mathexpr::log_info("test_reenumerate");

    std::vector<int> values{1, 2, 3};

    for(int pass = 0; pass < 2; pass++)
    {
        std::size_t expected_idx = 0;

        for(auto [idx, value] : mathexpr::enumerate(values))
        {
            (void)value;

            TEST_CHECK(idx == expected_idx, "Re-enumeration did not restart at zero");

            expected_idx++;
        }

        TEST_CHECK(expected_idx == values.size(), "Re-enumeration visited the wrong count");
    }
}

// Nested enumeration must keep independent indices.
static void test_nested()
{
    mathexpr::log_info("test_nested");

    std::vector<int> outer{0, 1, 2};
    std::vector<int> inner{0, 1};

    std::size_t num_iters = 0;

    for(auto [outer_idx, outer_value] : mathexpr::enumerate(outer))
    {
        (void)outer_value;

        std::size_t expected_inner = 0;

        for(auto [inner_idx, inner_value] : mathexpr::enumerate(inner))
        {
            (void)inner_value;

            TEST_CHECK(inner_idx == expected_inner, "Nested inner index is wrong");
            TEST_CHECK(outer_idx < outer.size(), "Nested outer index is out of range");

            expected_inner++;
            num_iters++;
        }

        TEST_CHECK(expected_inner == inner.size(), "Nested inner loop count is wrong");
    }

    TEST_CHECK(num_iters == outer.size() * inner.size(), "Nested loop count is wrong");
}

// A C style array must enumerate correctly.
static void test_c_array()
{
    mathexpr::log_info("test_c_array");

    std::array<int, 4> values{9, 8, 7, 6};

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "std::array index is not ascending");
        TEST_CHECK(value == values[expected_idx], "std::array value mismatch");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == values.size(), "std::array loop did not visit every element");
}

// A bidirectional container must enumerate correctly.
static void test_list()
{
    mathexpr::log_info("test_list");

    std::list<std::string> values{"alpha", "beta", "gamma"};

    const std::vector<std::string> expected{"alpha", "beta", "gamma"};

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "std::list index is not ascending");
        TEST_CHECK(value == expected[expected_idx], "std::list value mismatch");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == expected.size(), "std::list loop did not visit every element");
}

// A deque must enumerate correctly.
static void test_deque()
{
    mathexpr::log_info("test_deque");

    std::deque<int> values{5, 10, 15, 20};

    std::size_t sum = 0;
    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "std::deque index is not ascending");

        sum += static_cast<std::size_t>(value);
        expected_idx++;
    }

    TEST_CHECK(sum == 50, "std::deque values were not visited correctly");
}

// An associative container must enumerate in its own order with dense indices.
static void test_map()
{
    mathexpr::log_info("test_map");

    std::map<std::string, int> values{{"a", 1}, {"b", 2}, {"c", 3}};

    std::size_t expected_idx = 0;

    for(auto [idx, entry] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "std::map index is not ascending");

        entry.second *= 10;

        expected_idx++;
    }

    TEST_CHECK(expected_idx == values.size(), "std::map loop did not visit every element");
    TEST_CHECK(values["a"] == 10, "std::map mutation failed");
    TEST_CHECK(values["b"] == 20, "std::map mutation failed");
    TEST_CHECK(values["c"] == 30, "std::map mutation failed");
}

// A set yields const elements but indices must still be dense.
static void test_set()
{
    mathexpr::log_info("test_set");

    std::set<int> values{30, 10, 20};

    const std::vector<int> expected{10, 20, 30};

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "std::set index is not ascending");
        TEST_CHECK(value == expected[expected_idx], "std::set is not in sorted order");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == expected.size(), "std::set loop did not visit every element");
}

// A std::string must enumerate its characters.
static void test_string()
{
    mathexpr::log_info("test_string");

    std::string value = "MathExpr";

    std::size_t expected_idx = 0;

    for(auto [idx, character] : mathexpr::enumerate(value))
    {
        TEST_CHECK(idx == expected_idx, "std::string index is not ascending");
        TEST_CHECK(character == value[expected_idx], "std::string character mismatch");

        expected_idx++;
    }

    TEST_CHECK(expected_idx == value.size(), "std::string loop did not visit every character");

    for(auto [idx, character] : mathexpr::enumerate(value))
    {
        (void)idx;

        character = 'x';
    }

    TEST_CHECK(value == "xxxxxxxx", "std::string mutation failed");
}

// A large container must not lose or repeat indices.
static void test_large_container()
{
    mathexpr::log_info("test_large_container");

    constexpr std::size_t NUM_ELEMS = 100000;

    std::vector<std::size_t> values(NUM_ELEMS);

    for(std::size_t i = 0; i < NUM_ELEMS; i++)
        values[i] = i * 3;

    std::size_t expected_idx = 0;
    std::size_t sum = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "Large container index is not ascending");

        sum += value;
        expected_idx++;
    }

    TEST_CHECK(expected_idx == NUM_ELEMS, "Large container loop did not visit every element");
    TEST_CHECK(sum == 3 * (NUM_ELEMS - 1) * NUM_ELEMS / 2, "Large container sum is wrong");
}

// A container of a non copyable type must still enumerate by reference.
static void test_non_copyable()
{
    mathexpr::log_info("test_non_copyable");

    struct NonCopyable
    {
        int value;

        NonCopyable(const int value) : value(value) {}

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;

        NonCopyable(NonCopyable&&) = default;
        NonCopyable& operator=(NonCopyable&&) = default;
    };

    std::vector<NonCopyable> values;

    values.emplace_back(1);
    values.emplace_back(2);
    values.emplace_back(3);

    std::size_t expected_idx = 0;

    for(auto [idx, value] : mathexpr::enumerate(values))
    {
        TEST_CHECK(idx == expected_idx, "Non copyable index is not ascending");
        TEST_CHECK(value.value == static_cast<int>(expected_idx) + 1,
                   "Non copyable value mismatch");

        value.value *= 2;

        expected_idx++;
    }

    TEST_CHECK(values[0].value == 2, "Non copyable mutation failed");
    TEST_CHECK(values[2].value == 6, "Non copyable mutation failed");
}

// The iterator must satisfy the forward iterator requirements it advertises.
static void test_iterator_traits()
{
    mathexpr::log_info("test_iterator_traits");

    using Wrapper = decltype(mathexpr::enumerate(std::declval<std::vector<int>&>()));
    using Iterator = typename Wrapper::iterator;
    using Traits = std::iterator_traits<Iterator>;

    static_assert(std::is_same_v<typename Traits::iterator_category, std::forward_iterator_tag>,
                  "Enumerator must advertise the forward iterator category");
    static_assert(std::is_copy_constructible_v<Iterator>,
                  "Forward iterators must be copy constructible");
    static_assert(std::is_copy_assignable_v<Iterator>,
                  "Forward iterators must be copy assignable");
    static_assert(std::is_destructible_v<Iterator>,
                  "Forward iterators must be destructible");

    mathexpr::log_debug("Enumerator traits are consistent");
}

// std::distance must agree with the container size.
static void test_std_distance()
{
    mathexpr::log_info("test_std_distance");

    std::vector<int> values{1, 2, 3, 4, 5, 6};

    auto wrapper = mathexpr::enumerate(values);

    const auto dist = std::distance(wrapper.begin(), wrapper.end());

    TEST_CHECK(static_cast<std::size_t>(dist) == values.size(),
               "std::distance does not match the container size");
}

// Enumerating a subrange via a manually built iterator pair must work.
static void test_manual_iteration()
{
    mathexpr::log_info("test_manual_iteration");

    std::vector<int> values{0, 1, 2, 3, 4};

    auto wrapper = mathexpr::enumerate(values);

    auto it = wrapper.begin();
    const auto last = wrapper.end();

    std::size_t num_iters = 0;

    while(it != last)
    {
        auto pair = *it;

        TEST_CHECK(pair.first == num_iters, "Manual iteration index is wrong");
        TEST_CHECK(pair.second == static_cast<int>(num_iters), "Manual iteration value is wrong");

        ++it;
        num_iters++;
    }

    TEST_CHECK(num_iters == values.size(), "Manual iteration visited the wrong count");
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    mathexpr::set_log_level(mathexpr::LogLevel::Info);
    mathexpr::log_info("Starting enumerate test");

    test_basic_indices();
    test_empty_container();
    test_single_element();
    test_mutation();
    test_mutation_strings();
    test_reference_identity();
    test_const_container();
    test_const_ref_to_mutable();
    test_explicit_const_iterators();
    test_post_increment();
    test_pre_increment();
    test_iterator_copy_independence();
    test_repeated_dereference();
    test_reenumerate();
    test_nested();
    test_c_array();
    test_list();
    test_deque();
    test_map();
    test_set();
    test_string();
    test_large_container();
    test_non_copyable();
    test_iterator_traits();
    test_std_distance();
    test_manual_iteration();

    mathexpr::log_info("Finished enumerate test");

    return 0;
}