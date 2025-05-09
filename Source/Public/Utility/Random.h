#include "Core/Common.h"
#include <numeric>
#include <limits>
#include <random>

namespace aby::util {

    template <typename T, T Seed = 0, T Min = std::numeric_limits<T>::min(), T Max = std::numeric_limits<T>::max()> requires (std::is_arithmetic_v<T>)
    class Random {
    public:
        static T gen() noexcept {
            static std::mt19937_64 generator;
            static std::once_flag flag;

            std::call_once(flag, [&]() {
                if constexpr (Seed != 0) {
                    generator.seed(Seed);
                }
                else {
                    generator.seed(std::random_device{}());
                }
                });

            if constexpr (std::is_floating_point_v<T>) {
                std::uniform_real_distribution<T> distribution(Min, Max);
                return distribution(generator);
            }
            else if constexpr (std::is_integral_v<T>) {
                std::uniform_int_distribution<T> distribution(Min, Max);
                return distribution(generator);
            }
        }

    };

    class UUID {
    public:
        UUID();
        UUID(const UUID&) = default;
        UUID(UUID&&) noexcept = default;

        operator u64() const;
        operator std::string() const;
        
        UUID& operator=(const UUID&) = default;
        bool operator==(const UUID& other) const;
        bool operator!=(const UUID& other) const;
    private:
        uint64_t m_Value;
    };
}
