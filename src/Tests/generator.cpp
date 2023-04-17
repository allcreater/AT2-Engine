#include <gtest/gtest.h>
#include <generator.hpp>
#include <algorithm>
#include <string_view>

generator<char> GenerateAlphabet() {
    for (char c = 'a'; c <= 'z'; ++c) {
        co_yield c;
    }
}

TEST(Generator, GenerateAlphabet) {
     constexpr std::string_view alphabet = "abcdefghijklmnopqrstuvwxyz";

     auto range = GenerateAlphabet();
     ASSERT_TRUE(std::equal(alphabet.begin(), alphabet.end(), range.begin()));
}

TEST(Generator, Exception)
{
	auto gen = []() -> generator<int>
	{
		co_yield 1;
		throw std::runtime_error("error");
		co_yield 2;
	}();

	auto it = gen.begin();
	ASSERT_EQ(*it, 1);
	ASSERT_THROW(++it, std::runtime_error);
}	


int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
