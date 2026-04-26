#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::views;

namespace {

std::string ShapeToString(const Shape &shape) {
    return std::visit([](const auto &s) { return std::format("{}", s); }, shape);
}

bool SupportsIntersection(const Shape &lhs, const Shape &rhs) {
    return std::visit(
        [](const auto &a, const auto &b) {
            using A = std::decay_t<decltype(a)>;
            using B = std::decay_t<decltype(b)>;

            return (std::is_same_v<A, Line> && std::is_same_v<B, Line>) ||
                   (std::is_same_v<A, Line> && std::is_same_v<B, Circle>) ||
                   (std::is_same_v<A, Circle> && std::is_same_v<B, Line>) ||
                   (std::is_same_v<A, Circle> && std::is_same_v<B, Circle>);
        },
        lhs, rhs);
}

std::vector<Point2D> ExtractVertices(const Shape &shape) {
    return std::visit(
        [](const auto &s) -> std::vector<Point2D> {
            auto vertices = s.Vertices();
            return std::vector<Point2D>(vertices.begin(), vertices.end());
        },
        shape);
}

}  // namespace

void PrintAllIntersections(const Shape &shape, std::span<const Shape> others) {
    std::println("\n=== Intersections ===");

    auto supported = others | views::filter([&](const Shape &other) {
                         return &other != &shape && SupportsIntersection(shape, other);
                     });

    for (const auto &other : supported) {
        geometry::intersections::GetIntersectPoint(shape, other)
            .transform([&](const Point2D &p) {
                std::println("Пересечение найдено в точке {} между фигурами {} и {}", p, ShapeToString(shape),
                             ShapeToString(other));
                return p;
            })
            .or_else([&]() -> std::optional<Point2D> {
                std::println("Фигуры {} и {} не пересекаются", ShapeToString(shape), ShapeToString(other));
                return std::nullopt;
            });
    }
}

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    for (const auto &shape : shapes | views::take(5)) {
        const double distance = geometry::queries::DistanceToPoint(shape, p);
        std::println("Расстояние от точки {} до фигуры {} равно {:.2f}", p, ShapeToString(shape), distance);
    }
}

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");

    const auto collisions = geometry::utils::FindAllCollisions(shapes);
    if (collisions.empty()) {
        std::println("Коллизии методом Bounding Box не найдены");
    } else {
        for (const auto &[lhs, rhs] : collisions) {
            std::println("Найдена коллизия: {} и {}", ShapeToString(lhs), ShapeToString(rhs));
        }
    }

    geometry::utils::FindHighestShape(shapes)
        .transform([&](size_t index) {
            std::println("Самая высокая фигура: #{} = {}, height = {:.2f}", index, ShapeToString(shapes[index]),
                         geometry::queries::GetHeight(shapes[index]));
            return index;
        })
        .or_else([&]() -> std::optional<size_t> {
            std::println("Невозможно найти самую высокую фигуру: список пуст");
            return std::nullopt;
        });

    for (size_t i : views::iota(size_t{0}, shapes.size())) {
        for (size_t j : views::iota(i + 1, shapes.size())) {
            auto distance = geometry::queries::DistanceBetweenShapes(shapes[i], shapes[j]);
            if (distance.has_value()) {
                std::println("Расстояние между фигурами {} и {} равно {:.2f}", ShapeToString(shapes[i]),
                             ShapeToString(shapes[j]), *distance);
                return;
            }
        }
    }

    std::println("Не найдено ни одной пары фигур, для которых поддерживается вычисление расстояния");
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    bool found = false;
    for (const auto &shape : shapes | views::filter([](const Shape &s) {
                                 return geometry::queries::GetHeight(s) > 50.0;
                             }) | views::take(3)) {
        found = true;
        std::println("Фигура выше 50.0: {} (height = {:.2f})", ShapeToString(shape),
                     geometry::queries::GetHeight(shape));
    }

    if (!found) {
        std::println("Фигуры выше 50.0 не найдены");
    }

    if (shapes.empty()) {
        std::println("Список фигур пуст");
        return;
    }

    const auto [min_it, max_it] =
        rng::minmax_element(shapes, std::less{}, [](const Shape &s) { return geometry::queries::GetHeight(s); });

    std::println("Фигура с минимальной высотой: {} (height = {:.2f})", ShapeToString(*min_it),
                 geometry::queries::GetHeight(*min_it));
    std::println("Фигура с максимальной высотой: {} (height = {:.2f})", ShapeToString(*max_it),
                 geometry::queries::GetHeight(*max_it));
}

int main() {
    std::vector<Shape> shapes = utils::ParseShapes("circle 0 0 1.5; line 1 2 3 4; polygon 0 0 2 5; triangle 0 0 1 0 "
                                                   "0.5 1; polygon 0 0 1 2; badshape; circle 0 0 -1");
    std::println("Parsed {} shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту
    for (size_t i : views::iota(size_t{0}, shapes.size())) {
        std::println("#{}: {} | height = {:.2f}", i, ShapeToString(shapes[i]), queries::GetHeight(shapes[i]));
    }

    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(shapes[0], shapes);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(shapes);

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;

    for (const auto &shape : shapes) {
        auto vertices = ExtractVertices(shape);
        points.insert(points.end(), vertices.begin(), vertices.end());
    }

    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //
    auto hull_points = geometry::convex_hull::GrahamScan(points);
    if (!hull_points.empty()) {
        shapes.emplace_back(Polygon{std::move(hull_points)});
        std::println("\nConvex hull added as Polygon");
    } else {
        std::println("\nConvex hull was not built");
    }

    geometry::visualization::Draw(shapes);

    //
    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    //

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
        const auto delaunay = geometry::triangulation::DelaunayTriangulation(points);

        std::println("\n=== Delaunay triangulation ===");
        if (delaunay.empty()) {
            std::println("Триангуляция не построена");
        } else {
            std::vector<Shape> triangulated_shapes;
            triangulated_shapes.reserve(delaunay.size());

            for (const auto &triangle : delaunay) {
                std::println("{}", triangle);
                triangulated_shapes.emplace_back(Triangle{triangle.a, triangle.b, triangle.c});
            }

            geometry::visualization::Draw(triangulated_shapes);
        }
    }

    return 0;
}
