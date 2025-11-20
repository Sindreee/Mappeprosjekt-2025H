#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <utility>

namespace rnd {

// Shared PRNG engine for the whole program
inline std::mt19937& engine() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

inline void seed(uint32_t s) {
    engine().seed(s);
}

inline float uniform(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine());
}

inline std::pair<float, float> uniform2(float min, float max) {
    return {uniform(min, max), uniform(min, max)};
}

inline float unit() { // [0,1)
    return uniform(0.f, 1.f);
}

} // namespace rnd

#endif // RANDOM_HPP
