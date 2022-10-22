//
// Created by Kiril on 11.10.2022.
//

#ifndef DEEPLOM_ARRAY_IMPL_HPP
#define DEEPLOM_ARRAY_IMPL_HPP

#include <array>
#include <vector>

#include <common.hpp>
#include <Misc/type_traits.hpp>

namespace gorilla::implementations
{
    template<typename T, int size>
    struct _array
    {
        static_assert(size > 0);
        using type = std::array<T, size>;
    };

    template<typename T>
    struct _array<T, Dynamic>
    {
        using type = std::vector<T>;
    };

    template<typename T>
    struct _array<T, Infinite>
    {
        static_assert(type_traits::always_false<T>, "Arrays of Infinite Size are Not Implemented");
    };

} // gorilla

#endif //DEEPLOM_ARRAY_IMPL_HPP
