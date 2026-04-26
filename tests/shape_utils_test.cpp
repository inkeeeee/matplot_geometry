#include "shape_utils.hpp"
#include <gtest/gtest.h>

#include <variant>

TEST(ShapeUtilsTest, ParseShapesParsesOnlyValidShapes) {
    const auto shapes =
        geometry::utils::ParseShapes("circle 0 0 1.5; line 1 2 3 4; polygon 0 0 2 5; "
                                     "triangle 0 0 1 0 0.5 1; polygon 0 0 1 2; badshape; circle 0 0 -1");

    ASSERT_EQ(shapes.size(), 4u);
    EXPECT_TRUE(std::holds_alternative<geometry::Circle>(shapes[0]));
    EXPECT_TRUE(std::holds_alternative<geometry::Line>(shapes[1]));
    EXPECT_TRUE(std::holds_alternative<geometry::RegularPolygon>(shapes[2]));
    EXPECT_TRUE(std::holds_alternative<geometry::Triangle>(shapes[3]));
}

TEST(ShapeUtilsTest, FindAllCollisionsFindsBoundingBoxPairs) {
    const std::vector<geometry::Shape> shapes{geometry::Circle{{0.0, 0.0}, 2.0},
                                              geometry::Rectangle{{1.0, 1.0}, 2.0, 2.0},
                                              geometry::Line{{10.0, 10.0}, {12.0, 12.0}}};

    const auto collisions = geometry::utils::FindAllCollisions(shapes);
    ASSERT_EQ(collisions.size(), 1u);

    EXPECT_TRUE(std::holds_alternative<geometry::Circle>(collisions[0].first));
    EXPECT_TRUE(std::holds_alternative<geometry::Rectangle>(collisions[0].second));
}

TEST(ShapeUtilsTest, FindHighestShapeReturnsIndexOfHighestShape) {
    const std::vector<geometry::Shape> shapes{geometry::Triangle{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}},
                                              geometry::Line{{0.0, 0.0}, {0.0, 5.0}},
                                              geometry::Circle{{0.0, 0.0}, 2.0}};

    const auto index = geometry::utils::FindHighestShape(shapes);
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(*index, 1u);
}

TEST(ShapeUtilsTest, FindHighestShapeReturnsNulloptForEmptyInput) {
    const std::vector<geometry::Shape> shapes;
    EXPECT_FALSE(geometry::utils::FindHighestShape(shapes).has_value());
}
