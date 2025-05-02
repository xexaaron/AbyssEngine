#include "Utility/Random.h"

namespace aby::util {
    UUID::UUID() : m_Value(Random<uint64_t, 2083231>::gen()) {}

    UUID::operator std::uint64_t() const {
        return m_Value;
    }

    bool UUID::operator==(const UUID& other) const {
        return m_Value == other.m_Value;
    }

    bool UUID::operator!=(const UUID& other) const {
        return !this->operator==(other);
    }
}