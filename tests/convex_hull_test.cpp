#include "convex_hull.hpp"
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace {
constexpr double kEps = 1e-9;

bool ContainsPoint(const std::vector<geometry::Point2D> &points, const geometry::Point2D &target) {
    return std::ranges::any_of(points, [&](const geometry::Point2D &p) {
        return std::abs(p.x - target.x) < kEps && std::abs(p.y - target.y) < kEps;
    });
}
}  // namespace

TEST(ConvexHullTest, CrossProductWorks) {
    EXPECT_DOUBLE_EQ(geometry::convex_hull::CrossProduct({1.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}), 1.0);
}

TEST(ConvexHullTest, StackForGrahamScanWorks) {
    geometry::convex_hull::StackForGrahamScan stack;
    stack.Push({0.0, 0.0});
    stack.Push({1.0, 0.0});
    stack.Push({1.0, 1.0});

    EXPECT_EQ(stack.Size(), 3u);
    EXPECT_DOUBLE_EQ(stack.Top().x, 1.0);
    EXPECT_DOUBLE_EQ(stack.Top().y, 1.0);
    EXPECT_DOUBLE_EQ(stack.NextToTop().x, 1.0);
    EXPECT_DOUBLE_EQ(stack.NextToTop().y, 0.0);

    stack.Pop();
    EXPECT_EQ(stack.Size(), 2u);
}

TEST(ConvexHullTest, GrahamScanReturnsHullForSquareWithInnerPoint) {
    std::vector<geometry::Point2D> points{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}, {0.5, 0.5}};

    const auto hull = geometry::convex_hull::GrahamScan(points);

    ASSERT_EQ(hull.size(), 4u);
    EXPECT_TRUE(ContainsPoint(hull, {0.0, 0.0}));
    EXPECT_TRUE(ContainsPoint(hull, {1.0, 0.0}));
    EXPECT_TRUE(ContainsPoint(hull, {1.0, 1.0}));
    EXPECT_TRUE(ContainsPoint(hull, {0.0, 1.0}));
}

TEST(ConvexHullTest, GrahamScanReturnsEmptyForTooFewPoints) {
    std::vector<geometry::Point2D> points{{0.0, 0.0}, {1.0, 1.0}};
    EXPECT_TRUE(geometry::convex_hull::GrahamScan(points).empty());
}
