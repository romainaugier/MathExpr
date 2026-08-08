// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_SLAB)
#define __MATHEXPR_SLAB

#include "mathexpr/utils/memory.hpp"

#include <functional>

MATHEXPR_NAMESPACE_BEGIN

class SlabAllocator
{
    template<typename T>
    struct SlabNode
    {
        alignas(T) std::byte data[sizeof(T)];

        T* data_ptr() noexcept { return reinterpret_cast<T*>(std::addressof(this->data)); }
    };

    struct Slab 
    {
        std::byte* base;

        std::size_t sz;
        std::size_t offset;

        std::vector<std::function<void()>> dtors;

        Slab(const std::size_t sz) : sz(sz), offset(0)
        {
            this->base = new std::byte[sz];
        }

        ~Slab() 
        {
            if(this->base != nullptr)
            {
                this->clear();

                delete[] this->base;
            }
        }

        MATHEXPR_NON_COPYABLE(Slab);

        Slab(Slab&& other) noexcept : base(other.base),
                                      sz(other.sz),
                                      offset(other.offset),
                                      dtors(std::move(other.dtors))
        {
            other.base = nullptr;
            other.sz = 0;
            other.offset = 0;
        }

        Slab& operator=(Slab&& other) noexcept 
        {
            if(this != std::addressof(other))
            {
                if(this->base != nullptr)
                {
                    this->clear();
                    delete[] this->base;
                }

                this->base = other.base;
                this->sz = other.sz;
                this->offset = other.offset;
                this->dtors = std::move(other.dtors);

                other.base = nullptr;
                other.sz = 0;
                other.offset = 0;
            }

            return *this;
        }

        MATHEXPR_FORCE_INLINE bool is_full() const noexcept { return this->offset >= this->sz; }

        template<typename T, typename... Args>
        MATHEXPR_FORCE_INLINE T* emplace(Args&&... args) noexcept
        {
            MATHEXPR_ASSERT(this->base != nullptr, "base ptr is null");
            MATHEXPR_ASSERT(!this->is_full(), "Slab is full");

            constexpr std::size_t total_sz = sizeof(SlabNode<T>);

            if((this->offset + total_sz) > this->sz)
                return nullptr;

            SlabNode<T>* new_address = reinterpret_cast<SlabNode<T>*>((this->base + this->offset));

            std::construct_at(new_address->data_ptr(), std::forward<Args>(args)...);

            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                dtors.emplace_back([new_address]() {
                    new_address->data_ptr()->~T();
                });
            }

            this->offset += total_sz;

            return new_address->data_ptr();
        }

        void clear() noexcept
        {
            MATHEXPR_ASSERT(this->base != nullptr, "base ptr is null");
            
            for(auto& dtor : this->dtors)
                dtor();

            this->dtors.clear();

            this->offset = 0;
        }
    };

    std::vector<Slab> _slabs;

    std::size_t _slab_sz;

public:
    static constexpr std::size_t DEFAULT_SLAB_SZ = 1_Kb;

    SlabAllocator(const std::size_t slab_sz = DEFAULT_SLAB_SZ) : _slab_sz(slab_sz)
    {
        this->_slabs.emplace_back(slab_sz);
    }

    template<typename T, typename... Args>
    MATHEXPR_FORCE_INLINE T* allocate(Args&&... args) noexcept
    {
        for(Slab& slab : this->_slabs)
        {
            if(slab.is_full())
                continue;

            if(T* p = slab.emplace<T>(std::forward<Args>(args)...))
                return p;
        }

        Slab& slab = this->_slabs.emplace_back(this->_slab_sz);

        return slab.emplace<T>(std::forward<Args>(args)...);
    }

    void clear() noexcept
    {
        for(Slab& slab : this->_slabs)
            slab.clear();
    }
};

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_SLAB)