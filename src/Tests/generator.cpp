#include <gtest/gtest.h>
#include <generator.hpp>
#include <algorithm>

generator<char> GenerateAlphabet() {
    for (char c = 'a'; c <= 'z'; ++c) {
        co_yield c;
    }
}

TEST(Generator, GenerateAlphabet) {
    // constexpr auto alphabet = "abcdefghijklmnopqrstuvwxyz";

    // auto range = GenerateAlphabet();
    // ASSERT_TRUE(std::equal(range.begin(), range.end(), alphabet.begin()));
}