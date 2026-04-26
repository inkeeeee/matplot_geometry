#include "geometry.hpp"
#include <gtest/gtest.h>

#include <cmath>
#include <format>
#include <string>
#include <vector>

namespace {
constexpr double kEps = 1e-9;

void ExpectPointNear(const geometry::Point2D &actual, const geometry::Point2D &expected, double eps = kEps) {
    EXPECT_NEAR(actual.x, expected.x, eps);
    EXPECT_NEAR(actual.y, expected.y, eps);
}
}  // namespace

TEST(Point2DTest, ArithmeticAndGeometryOperationsWork) {
    geometry::Point2D a{3.0, 4.0};
    geometry::Point2D b{1.0, 2.0};

    ExpectPointNear(a + b, {4.0, 6.0});
    ExpectPointNear(a - b, {2.0, 2.0});
    ExpectPointNear(a * 2.0, {6.0, 8.0});
    ExpectPointNear(a / 2.0, {1.5, 2.0});

    EXPECT_DOUBLE_EQ(a.Dot(b), 11.0);
    EXPECT_DOUBLE_EQ(a.Cross(b), 2.0);
    EXPECT_NEAR(a.Length(), 5.0, kEps);
    EXPECT_NEAR(a.DistanceTo({0.0, 0.0}), 5.0, kEps);

    const auto norm = a.Normalize();
    EXPECT_NEAR(norm.Length(), 1.0, kEps);
    ExpectPointNear(norm, {0.6, 0.8});
}

TEST(Point2DTest, ComparisonOperatorsWork) {
    EXPECT_TRUE((geometry::Point2D{1.0, 1.0} < geometry::Point2D{2.0, 2.0}));
    EXPECT_FALSE((geometry::Point2D{1.0, 3.0} < geometry::Point2D{2.0, 2.0}));
    EXPECT_TRUE((geometry::Point2D{1.0, 2.0} == geometry::Point2D{1.0, 2.0}));
    EXPECT_FALSE((geometry::Point2D{1.0, 2.0} == geometry::Point2D{2.0, 1.0}));
}

TEST(Lines2DDynTest, ReservePushBackAndFrontWork) {
    geometry::Lines2DDyn lines;
    lines.Reserve(2);
    lines.PushBack(geometry::Point2D{1.0, 2.0});
    lines.PushBack(3.0, 4.0);

    ASSERT_EQ(lines.x.size(), 2u);
    ASSERT_EQ(lines.y.size(), 2u);
    ExpectPointNear(lines.Front(), {1.0, 2.0});
}

TEST(BoundingBoxTest, OverlapsAndDimensionsWork) {
    geometry::BoundingBox a{0.0, 0.0, 2.0, 3.0};
    geometry::BoundingBox b{1.0, 1.0, 4.0, 5.0};
    geometry::BoundingBox c{3.1, 0.0, 4.0, 1.0};

    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_FALSE(a.Overlaps(c));
    EXPECT_DOUBLE_EQ(a.Width(), 2.0);
    EXPECT_DOUBLE_EQ(a.Height(), 3.0);
    ExpectPointNear(a.Center(), {1.0, 1.5});
}

TEST(LineTest, MethodsWork) {
    geometry::Line line{{0.0, 0.0}, {3.0, 4.0}};

    EXPECT_NEAR(line.Length(), 5.0, kEps);
    ExpectPointNear(line.Direction(), {0.6, 0.8});
    ExpectPointNear(line.Center(), {1.5, 2.0});
    EXPECT_DOUBLE_EQ(line.Height(), 4.0);

    const auto box = line.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, 0.0);
    EXPECT_DOUBLE_EQ(box.min_y, 0.0);
    EXPECT_DOUBLE_EQ(box.max_x, 3.0);
    EXPECT_DOUBLE_EQ(box.max_y, 4.0);

    const auto vertices = line.Vertices();
    ASSERT_EQ(vertices.size(), 2u);
    ExpectPointNear(vertices[0], {0.0, 0.0});
    ExpectPointNear(vertices[1], {3.0, 4.0});

    const auto lines = line.Lines();
    ASSERT_EQ(lines.x.size(), 2u);
    ASSERT_EQ(lines.y.size(), 2u);
    EXPECT_DOUBLE_EQ(lines.x[0], 0.0);
    EXPECT_DOUBLE_EQ(lines.x[1], 3.0);
}

TEST(TriangleTest, MethodsWork) {
    geometry::Triangle triangle{{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}};

    EXPECT_NEAR(triangle.Area(), 2.0, kEps);
    EXPECT_DOUBLE_EQ(triangle.Height(), 2.0);
    ExpectPointNear(triangle.Center(), {2.0 / 3.0, 2.0 / 3.0});

    const auto box = triangle.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, 0.0);
    EXPECT_DOUBLE_EQ(box.min_y, 0.0);
    EXPECT_DOUBLE_EQ(box.max_x, 2.0);
    EXPECT_DOUBLE_EQ(box.max_y, 2.0);

    const auto vertices = triangle.Vertices();
    ASSERT_EQ(vertices.size(), 3u);

    const auto lines = triangle.Lines();
    ASSERT_EQ(lines.x.size(), 4u);
    ASSERT_EQ(lines.y.size(), 4u);
    EXPECT_DOUBLE_EQ(lines.x.front(), lines.x.back());
    EXPECT_DOUBLE_EQ(lines.y.front(), lines.y.back());
}

TEST(RectangleTest, MethodsWork) {
    geometry::Rectangle rect{{1.0, 2.0}, 3.0, 4.0};

    ExpectPointNear(rect.TopRight(), {4.0, 6.0});
    ExpectPointNear(rect.Center(), {2.5, 4.0});
    EXPECT_DOUBLE_EQ(rect.Height(), 6.0);

    const auto box = rect.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, 1.0);
    EXPECT_DOUBLE_EQ(box.min_y, 2.0);
    EXPECT_DOUBLE_EQ(box.max_x, 4.0);
    EXPECT_DOUBLE_EQ(box.max_y, 6.0);

    const auto vertices = rect.Vertices();
    ASSERT_EQ(vertices.size(), 4u);

    const auto lines = rect.Lines();
    ASSERT_EQ(lines.x.size(), 5u);
    ASSERT_EQ(lines.y.size(), 5u);
    EXPECT_DOUBLE_EQ(lines.x.front(), lines.x.back());
    EXPECT_DOUBLE_EQ(lines.y.front(), lines.y.back());
}

TEST(RegularPolygonTest, MethodsWork) {
    geometry::RegularPolygon polygon{{0.0, 0.0}, 2.0, 4};

    const auto vertices = polygon.Vertices();
    ASSERT_EQ(vertices.size(), 4u);
    ExpectPointNear(vertices[0], {2.0, 0.0});

    const auto box = polygon.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, -2.0);
    EXPECT_DOUBLE_EQ(box.min_y, -2.0);
    EXPECT_DOUBLE_EQ(box.max_x, 2.0);
    EXPECT_DOUBLE_EQ(box.max_y, 2.0);

    EXPECT_DOUBLE_EQ(polygon.Height(), 2.0);
    ExpectPointNear(polygon.Center(), {0.0, 0.0});

    const auto lines = polygon.Lines();
    ASSERT_EQ(lines.x.size(), 5u);
    ASSERT_EQ(lines.y.size(), 5u);
    EXPECT_DOUBLE_EQ(lines.x.front(), lines.x.back());
    EXPECT_DOUBLE_EQ(lines.y.front(), lines.y.back());
}

TEST(CircleTest, MethodsWork) {
    geometry::Circle circle{{1.0, 2.0}, 3.0};

    const auto box = circle.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, -2.0);
    EXPECT_DOUBLE_EQ(box.min_y, -1.0);
    EXPECT_DOUBLE_EQ(box.max_x, 4.0);
    EXPECT_DOUBLE_EQ(box.max_y, 5.0);

    EXPECT_DOUBLE_EQ(circle.Height(), 5.0);
    ExpectPointNear(circle.Center(), {1.0, 2.0});

    const auto vertices = circle.Vertices(4);
    ASSERT_EQ(vertices.size(), 4u);
    ExpectPointNear(vertices[0], {4.0, 2.0});

    const auto lines = circle.Lines(8);
    ASSERT_EQ(lines.x.size(), 9u);
    ASSERT_EQ(lines.y.size(), 9u);
    EXPECT_DOUBLE_EQ(lines.x.front(), lines.x.back());
    EXPECT_DOUBLE_EQ(lines.y.front(), lines.y.back());
}

TEST(PolygonTest, MethodsWork) {
    geometry::Polygon polygon({{0.0, 0.0}, {3.0, 0.0}, {3.0, 2.0}, {0.0, 2.0}});

    const auto box = polygon.BoundBox();
    EXPECT_DOUBLE_EQ(box.min_x, 0.0);
    EXPECT_DOUBLE_EQ(box.min_y, 0.0);
    EXPECT_DOUBLE_EQ(box.max_x, 3.0);
    EXPECT_DOUBLE_EQ(box.max_y, 2.0);

    EXPECT_DOUBLE_EQ(polygon.Height(), 2.0);
    ExpectPointNear(polygon.Center(), {1.5, 1.0});

    const auto vertices = polygon.Vertices();
    ASSERT_EQ(vertices.size(), 4u);
    ExpectPointNear(vertices[2], {3.0, 2.0});

    const auto lines = polygon.Lines();
    ASSERT_EQ(lines.x.size(), 5u);
    ASSERT_EQ(lines.y.size(), 5u);
    EXPECT_DOUBLE_EQ(lines.x.front(), lines.x.back());
    EXPECT_DOUBLE_EQ(lines.y.front(), lines.y.back());
}

TEST(FormatterTest, PointAndVectorFormattersWork) {
    const geometry::Point2D p{1.0, 2.5};
    EXPECT_EQ(std::format("{}", p), "(1.00, 2.50)");

    const std::vector<geometry::Point2D> points{{1.0, 2.0}, {3.0, 4.0}};
    EXPECT_EQ(std::format("{}", points), "[(1.00, 2.00), (3.00, 4.00)]");
    EXPECT_EQ(std::format("{:new_line}", points), "[\n\t(1.00, 2.00)\n\t(3.00, 4.00)\n]");
}
