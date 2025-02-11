#pragma once
#include <numeric>
#include <utility>
#include <limits>
#include <iostream>
#include <algorithm>

namespace aby {

    template <typename T> requires (std::is_arithmetic_v<T>)
    class Number {
    public:
        Number() : m_Value(0) {}
        Number(const T& value) : m_Value(value) {}
        Number(T&& value) noexcept : m_Value(std::exchange(value, 0)) {}
        Number(const Number& other) : m_Value(other.m_Value) {}
        Number(Number&& other) noexcept : m_Value(std::exchange(other.m_Value, 0)) {}

        static constexpr T max() { return std::numeric_limits<T>::max() }
        static constexpr T min() { return std::numeric_limits<T>::lowest() };
        T& clamp(T minimum, T maximum) { m_Value = std::clamp(m_Value, minimum, maximum); return *this; }
        std::string to_string() { return std::to_string(m_Value); }

        Number& operator=(const T& value) { m_Value = value; return *this; }
        Number& operator=(T&& value) noexcept { m_Value = std::exchange(value, 0); return *this; }
        Number& operator=(const Number& other) { m_Value = other.m_Value; return *this; }
        Number& operator=(Number&& other) noexcept { m_Value = std::exchange(other.m_Value, 0); return *this; }

        Number operator+(const Number& other) const { return Number(m_Value + other.m_Value); }
        Number operator-(const Number& other) const { return Number(m_Value - other.m_Value); }
        Number operator*(const Number& other) const { return Number(m_Value * other.m_Value); }
        Number operator/(const Number& other) const { return Number(m_Value / other.m_Value); }
        Number operator%(const Number& other) const { return Number(m_Value % other.m_Value); }

        Number& operator+=(const Number& other) { m_Value += other.m_Value; return *this; }
        Number& operator-=(const Number& other) { m_Value -= other.m_Value; return *this; }
        Number& operator*=(const Number& other) { m_Value *= other.m_Value; return *this; }
        Number& operator/=(const Number& other) { m_Value /= other.m_Value; return *this; }
        Number& operator%=(const Number& other) { m_Value %= other.m_Value; return *this; }

        Number operator+() const { return *this; }
        Number operator-() const { return Number(-m_Value); }

        Number& operator++() { ++m_Value; return *this; }
        Number operator++(int) { Number temp = *this; ++m_Value; return temp; }
        Number& operator--() { --m_Value; return *this; }
        Number operator--(int) { Number temp = *this; --m_Value; return temp; }

        bool operator==(const Number& other) const { return m_Value == other.m_Value; }
        bool operator!=(const Number& other) const { return m_Value != other.m_Value; }
        bool operator<(const Number& other) const { return m_Value < other.m_Value; }
        bool operator>(const Number& other) const { return m_Value > other.m_Value; }
        bool operator<=(const Number& other) const { return m_Value <= other.m_Value; }
        bool operator>=(const Number& other) const { return m_Value >= other.m_Value; }
    
        explicit operator bool() const { return m_Value != false; }
        operator const T& () const { return m_Value; }
        operator T& () { return m_Value; }

        friend std::ostream& operator<<(std::ostream& os, const Number& num) { os << num.m_Value; return os; }
        friend std::istream& operator>>(std::istream& is, Number& num) { is >> m_Value; return is; }
    private:
        T m_Value;
    };

    using u8  = Number<std::uint8_t>;
    using u16 = Number<std::uint16_t>;
    using u32 = Number<std::uint32_t>;
    using u64 = Number<std::uint64_t>;
    using i8  = Number<std::int8_t>;
    using i16 = Number<std::int16_t>;
    using i32 = Number<std::int32_t>;
    using i64 = Number<std::int64_t>;
    using f32 = Number<float>;
    using f64 = Number<double>;

}