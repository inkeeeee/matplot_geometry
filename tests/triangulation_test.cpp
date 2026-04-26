#include "triangulation.hpp"
#include <gtest/gtest.h>

#include <vector>

TEST(TriangulationTest, DelaunayTriangleHelpersWork) {
    geometry::triangulation::DelaunayTriangle triangle{{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}};

    const auto center = triangle.Circumcenter();
    EXPECT_NEAR(center.x, 1.0, 1e-9);
    EXPECT_NEAR(center.y, 1.0, 1e-9);
    EXPECT_NEAR(triangle.Circumradius(), std::sqrt(2.0), 1e-9);
    EXPECT_TRUE(triangle.ContainsPoint({1.0, 1.0}));
}

TEST(TriangulationTest, EdgeOrderingAndEqualityWork) {
    geometry::triangulation::Edge e1{{2.0, 2.0}, {1.0, 1.0}};
    geometry::triangulation::Edge e2{{1.0, 1.0}, {2.0, 2.0}};

    EXPECT_TRUE(e1 == e2);
    EXPECT_DOUBLE_EQ(e1.p1.x, 1.0);
    EXPECT_DOUBLE_EQ(e1.p1.y, 1.0);
}

TEST(TriangulationTest, DelaunayTriangulationReturnsEmptyForTooFewPoints) {
    const std::vector<geometry::Point2D> points{{0.0, 0.0}, {1.0, 1.0}};
    EXPECT_TRUE(geometry::triangulation::DelaunayTriangulation(points).empty());
}

TEST(TriangulationTest, DelaunayTriangulationBuildsSingleTriangle) {
    const std::vector<geometry::Point2D> points{{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}};
    const auto triangles = geometry::triangulation::DelaunayTriangulation(points);

    ASSERT_EQ(triangles.size(), 1u);
    const auto vertices = triangles[0].vertices();
    ASSERT_EQ(vertices.size(), 3u);
}

TEST(TriangulationTest, FormatterWorks) {
    const geometry::triangulation::DelaunayTriangle triangle{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}};
    const auto text = std::format("{}", triangle);

    EXPECT_NE(text.find("DelaunayTriangle"), std::string::npos);
    EXPECT_NE(text.find("(0.00, 0.00)"), std::string::npos);
}
