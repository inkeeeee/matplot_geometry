#include "queries.hpp"
#include <gtest/gtest.h>

namespace {
constexpr double kEps = 1e-9;
}

TEST(QueriesTest, DistanceToPointForCircleWorks) {
    geometry::Shape circle = geometry::Circle{{0.0, 0.0}, 2.0};
    const double distance = geometry::queries::DistanceToPoint(circle, {5.0, 0.0});
    EXPECT_NEAR(distance, 3.0, kEps);
}

TEST(QueriesTest, DistanceToPointForLineWorks) {
    geometry::Shape line = geometry::Line{{0.0, 0.0}, {4.0, 0.0}};
    const double distance = geometry::queries::DistanceToPoint(line, {2.0, 3.0});
    EXPECT_NEAR(distance, 3.0, kEps);
}

TEST(QueriesTest, GetBoundBoxWorksForRectangle) {
    geometry::Shape rect = geometry::Rectangle{{1.0, 2.0}, 3.0, 4.0};
    const auto box = geometry::queries::GetBoundBox(rect);

    EXPECT_DOUBLE_EQ(box.min_x, 1.0);
    EXPECT_DOUBLE_EQ(box.min_y, 2.0);
    EXPECT_DOUBLE_EQ(box.max_x, 4.0);
    EXPECT_DOUBLE_EQ(box.max_y, 6.0);
}

TEST(QueriesTest, GetHeightWorksForLine) {
    geometry::Shape line = geometry::Line{{1.0, 2.0}, {3.0, 4.0}};
    EXPECT_DOUBLE_EQ(geometry::queries::GetHeight(line), 4.0);
}

TEST(QueriesTest, BoundingBoxesOverlapWorks) {
    geometry::Shape a = geometry::Circle{{0.0, 0.0}, 2.0};
    geometry::Shape b = geometry::Rectangle{{1.0, 1.0}, 2.0, 2.0};
    geometry::Shape c = geometry::Rectangle{{10.0, 10.0}, 1.0, 1.0};

    EXPECT_TRUE(geometry::queries::BoundingBoxesOverlap(a, b));
    EXPECT_FALSE(geometry::queries::BoundingBoxesOverlap(a, c));
}

TEST(QueriesTest, DistanceBetweenShapesWorksForCircles) {
    geometry::Shape c1 = geometry::Circle{{0.0, 0.0}, 1.0};
    geometry::Shape c2 = geometry::Circle{{5.0, 0.0}, 1.0};

    const auto distance = geometry::queries::DistanceBetweenShapes(c1, c2);
    ASSERT_TRUE(distance.has_value());
    EXPECT_NEAR(*distance, 3.0, kEps);
}

TEST(QueriesTest, DistanceBetweenShapesWorksForParallelLines) {
    geometry::Shape l1 = geometry::Line{{0.0, 0.0}, {2.0, 0.0}};
    geometry::Shape l2 = geometry::Line{{0.0, 3.0}, {2.0, 3.0}};

    const auto distance = geometry::queries::DistanceBetweenShapes(l1, l2);
    ASSERT_TRUE(distance.has_value());
    EXPECT_NEAR(*distance, 3.0, kEps);
}

TEST(QueriesTest, DistanceBetweenShapesReturnsNulloptForUnsupportedPairs) {
    geometry::Shape line = geometry::Line{{0.0, 0.0}, {1.0, 0.0}};
    geometry::Shape rect = geometry::Rectangle{{0.0, 0.0}, 2.0, 2.0};

    EXPECT_FALSE(geometry::queries::DistanceBetweenShapes(line, rect).has_value());
}
