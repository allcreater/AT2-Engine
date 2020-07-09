#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AT2/AT2.h>

using namespace AT2;


TEST(AABB_2d, InvalidAfterInvalidating)
{
    AABB2d aabb;

    aabb.Reset(false);
    ASSERT_TRUE(aabb.Valid() && aabb.Empty());

    aabb.Reset(true);
    ASSERT_FALSE(aabb.Valid());
}

TEST(AABB_2d, ExtensionWorksCorrectly)
{
    const AABB2d aabb(glm::vec2(-10, -10), glm::vec2(10, 10));

    //points inside
    ASSERT_EQ(aabb.GetExtended(glm::vec2(-5, 0)), aabb);
    ASSERT_EQ(aabb.GetExtended(glm::vec2(5, 0)), aabb);
    ASSERT_EQ(aabb.GetExtended(glm::vec2(0, -5)), aabb);
    ASSERT_EQ(aabb.GetExtended(glm::vec2(0, 5)), aabb);

    //points outside
    ASSERT_EQ(aabb.GetExtended(glm::vec2(-20, 0)), AABB2d(glm::vec2(-20, -10), glm::vec2(10, 10)));
    ASSERT_EQ(aabb.GetExtended(glm::vec2(20, 0)), AABB2d(glm::vec2(-10, -10), glm::vec2(20, 10)));
    ASSERT_EQ(aabb.GetExtended(glm::vec2(0, -20)), AABB2d(glm::vec2(-10, -20), glm::vec2(10, 10)));
    ASSERT_EQ(aabb.GetExtended(glm::vec2(0, 20)), AABB2d(glm::vec2(-10, -10), glm::vec2(10, 20)));
}

TEST(AABB_2d, UnionWorksCorrectly)
{
    const AABB2d expectedUnion(glm::vec2(90, 100), glm::vec2(200, 400));

    AABB2d a(glm::vec2(100, 200), glm::vec2(200, 400)), b(glm::vec2(90, 100), glm::vec2(150, 400));

    ASSERT_EQ(a.GetUnion(b), expectedUnion);
}

TEST(AABB_2d, IsPointInsideTest)
{
    const AABB2d aabb(glm::vec2(-10, -10), glm::vec2(10, 10));

    ASSERT_FALSE(aabb.IsPointInside(glm::vec2(-20, 0)));
    ASSERT_FALSE(aabb.IsPointInside(glm::vec2(20, 0)));
    ASSERT_FALSE(aabb.IsPointInside(glm::vec2(0, -20)));
    ASSERT_FALSE(aabb.IsPointInside(glm::vec2(0, 20)));

    ASSERT_TRUE(aabb.IsPointInside(glm::vec2(0, 0)));
}

TEST(AABB_2d, IsWidthAndHeightCorrect)
{
    const AABB2d aabb1(glm::vec2(-10, -20), glm::vec2(10, 10));
    const AABB2d aabb2(glm::vec2(-20, -20), glm::vec2(-10, -10));
    const AABB2d aabb3(glm::vec2(10, 10), glm::vec2(30, 50));

    ASSERT_EQ(aabb1.GetWidth(), 20.0f);
    ASSERT_EQ(aabb1.GetHeight(), 30.0f);

    ASSERT_EQ(aabb2.GetWidth(), 10.0f);
    ASSERT_EQ(aabb2.GetHeight(), 10.0f);

    ASSERT_EQ(aabb3.GetWidth(), 20.0f);
    ASSERT_EQ(aabb3.GetHeight(), 40.0f);
}

TEST(AABB_2d, IsIntersectionWorksCorrectly)
{
    const AABB2d aabb1(glm::vec2(-10, -20), glm::vec2(10, 20));

    const AABB2d aabb2(glm::vec2(0, -30), glm::vec2(10, -10));

    ASSERT_EQ(aabb1.GetIntersection(AABB2d(aabb1)), aabb1);

    ASSERT_EQ(aabb1.GetIntersection(aabb2), AABB2d(glm::vec2(0, -20), glm::vec2(10, -10)));
}
