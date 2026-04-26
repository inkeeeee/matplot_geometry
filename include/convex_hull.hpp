#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <expected>
#include <ranges>
#include <stack>
#include <vector>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {
public:
    void Push(const Point2D &p) { s.push_back(p); }
    void Pop() { s.pop_back(); }

    size_t Size() { return s.size(); }
    Point2D Top() { return s.back(); }
    Point2D NextToTop() { return *std::prev(s.end(), 2); }

    std::vector<Point2D> &&Extract() & { return std::move(s); }

private:
    std::vector<Point2D> s;
};

namespace detail {

inline std::expected<std::vector<Point2D>, const char *> TryGrahamScan(std::span<Point2D> points) {
    if (points.size() < 3) {
        return std::unexpected("GrahamScan requires at least 3 points");
    }

    const auto pivot_it = std::ranges::min_element(points, {}, [](const Point2D &p) { return std::pair{p.y, p.x}; });

    std::ranges::iter_swap(points.begin(), pivot_it);
    const Point2D pivot = points.front();

    std::ranges::sort(points.subspan(1), [&](const Point2D &lhs, const Point2D &rhs) {
        const double cross = CrossProduct(lhs, pivot, rhs);
        if (cross == 0.0) {
            return pivot.DistanceTo(lhs) < pivot.DistanceTo(rhs);
        }
        return cross > 0.0;
    });

    StackForGrahamScan stack;
    stack.Push(points[0]);
    stack.Push(points[1]);
    stack.Push(points[2]);

    for (const auto &p : points | std::views::drop(3)) {
        while (stack.Size() >= 2 && CrossProduct(stack.NextToTop(), stack.Top(), p) >= 0.0) {
            stack.Pop();
        }
        stack.Push(p);
    }

    return stack.Extract();
}
}  // namespace detail

[[nodiscard]] inline std::vector<Point2D> GrahamScan(std::span<Point2D> points) noexcept {
    auto result = detail::TryGrahamScan(points);
    if (result.has_value()) {
        return std::move(*result);
    }
    return {};
}

}  // namespace geometry::convex_hull
