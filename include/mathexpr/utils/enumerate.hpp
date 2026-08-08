// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

// Clang 21 does not have std::views::enumerate so we write a little replacement here

#pragma once

#if !defined(__MATHEXPR_ENUMERATE)
#define __MATHEXPR_ENUMERATE

#include "mathexpr/common.hpp"

#include <iterator>
#include <utility>
#include <functional>

MATHEXPR_NAMESPACE_BEGIN

template<typename T>
using EnumeratePair = std::pair<std::size_t, T>;

template<typename Iterator>
class Enumerator
{
private:
    Iterator _iterator;
    std::size_t _idx;

public:
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using value_type = EnumeratePair<typename std::iterator_traits<Iterator>::reference>;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::forward_iterator_tag;

    Enumerator(Iterator iterator, std::size_t idx) : _iterator(iterator),
                                                     _idx(idx) {}

    reference operator*() const noexcept
    {
        return reference(this->_idx, *this->_iterator);
    }

    Enumerator& operator++() noexcept
    {
        ++this->_iterator;
        ++this->_idx;
        return *this;
    }

    Enumerator operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const Enumerator& other) const noexcept
    {
        return this->_iterator == other._iterator;
    }

    bool operator!=(const Enumerator& other) const noexcept
    {
        return this->_iterator != other._iterator;
    }
};

template<typename Container>
class EnumeratorWrapper
{
    Container& _container;

public:
    using iterator = Enumerator<decltype(std::declval<Container&>().begin())>;
    using const_iterator = Enumerator<decltype(std::declval<const Container&>().begin())>;

    EnumeratorWrapper(Container& container) : _container(container) {}

    iterator begin()
    {
        return iterator(this->_container.begin(), 0);
    }

    iterator end()
    {
        return iterator(this->_container.end(), this->_container.size());
    }

    const_iterator begin() const
    {
        return const_iterator(this->_container.begin(), 0);
    }

    const_iterator end() const
    {
        return const_iterator(this->_container.end(), this->_container.size());
    }

    const_iterator cbegin() const
    {
        return const_iterator(this->_container.begin(), 0);
    }

    const_iterator cend() const
    {
        return const_iterator(this->_container.end(), this->_container.size());
    }
};

template<typename Container>
EnumeratorWrapper<Container> enumerate(Container& container) noexcept
{
    return EnumeratorWrapper<Container>(container);
}

template<typename Container>
EnumeratorWrapper<const Container> enumerate(const Container& container) noexcept
{
    return EnumeratorWrapper<const Container>(container);
}

template<typename Container>
EnumeratorWrapper<const Container> enumerate(const Container&&) = delete;

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_ENUMERATE)
