#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

//
// Utility concepts for C++20/23
//
// Author: Kleber Kruger
// Date: 2025-08-09
//

namespace util::concepts
{

// -------------------------
// Basic types
// -------------------------

/// Checks if T is the same type as U
template<typename T, typename U>
concept SameAs = std::same_as<T, U>;

/// Checks if T is derived from Base
template<typename T, typename Base>
concept DerivedFrom = std::derived_from<T, Base>;

/// Checks if T is an integral type
template<typename T>
concept Integral = std::integral<T>;

/// Checks if T is a floating point type
template<typename T>
concept FloatingPoint = std::floating_point<T>;

// -------------------------
// Ranges
// -------------------------

/// Range whose elements are exactly of type T
template<typename R, typename T>
concept RangeOf = std::ranges::input_range<R> &&
                  std::same_as<std::ranges::range_value_t<R>, T>;

/// Range whose elements are convertible to type T
template<typename R, typename T>
concept RangeOfConvertibleTo = std::ranges::input_range<R> &&
                               std::convertible_to<std::ranges::range_value_t<R>, T>;

// -------------------------
// Parameter packs
// -------------------------

/// All parameters are exactly of type T
template<typename T, typename... Args>
concept PackOf = (std::same_as<std::decay_t<Args>, T> && ...);

/// All parameters are convertible to type T
template<typename T, typename... Args>
concept PackConvertibleTo = (std::convertible_to<std::decay_t<Args>, T> && ...);

template<typename T, typename... Ts>
concept AnyOf = (std::same_as<T, Ts> || ...);

}// namespace util::concepts

// namespace concepts = util::concepts;