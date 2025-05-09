#include "Utility/Random.h"

namespace aby::util {
    UUID::UUID() : m_Value(Random<uint64_t, 2083231>::gen()) {}

    UUID::operator u64() const {
        return m_Value;
    }
    
    UUID::operator std::string() const {
        return std::to_string(m_Value);
    }

    bool UUID::operator==(const UUID& other) const {
        return m_Value == other.m_Value;
    }

    bool UUID::operator!=(const UUID& other) const {
        return !this->operator==(other);
    }
}