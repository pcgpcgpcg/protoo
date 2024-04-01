#include "utils.h"
#include <random>
#include <string>

using std::default_random_engine;
using std::uniform_real_distribution;

int generateRandomNumber(size_t length) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, 9);

    int result = 0;
    for (size_t i = 0; i < length; ++i) {
        // 生成一位数字并考虑其权重，累加到结果中
        result = result * 10 + distribution(generator);
    }

    return result;
}