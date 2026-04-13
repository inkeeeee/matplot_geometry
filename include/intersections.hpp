#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <variant>

namespace geometry::intersections {

class IntersectionVisitor {
public:
    [[nodiscard]] std::optional<Point2D> operator()(const Line &lhs, const Line &rhs) const {
        return IntersectLineLine(lhs, rhs);
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Line &line, const Circle &circle) const {
        return IntersectLineCircle(line, circle);
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Circle &circle, const Line &line) const {
        return IntersectLineCircle(line, circle);
    }

    [[nodiscard]] std::optional<Point2D> operator()(const Circle &lhs, const Circle &rhs) const {
        return IntersectCircleCircle(lhs, rhs);
    }

    template <class T, class U>
    [[nodiscard]] std::optional<Point2D> operator()(const T &, const U &) const {
        throw std::logic_error("Intersection is not supported for this pair of shapes");
    }

private:
    static constexpr double kEps = 1e-9;

    [[nodiscard]] static bool IsZero(double value) noexcept { return std::abs(value) <= kEps; }

    [[nodiscard]] static bool IsInUnitSegment(double t) noexcept { return t >= -kEps && t <= 1.0 + kEps; }

    [[nodiscard]] static bool SamePoint(const Point2D &a, const Point2D &b) noexcept {
        return IsZero(a.x - b.x) && IsZero(a.y - b.y);
    }

    [[nodiscard]] static Point2D LexicographicallyMin(const Point2D &a, const Point2D &b) noexcept {
        if (a.x < b.x - kEps) {
            return a;
        }
        if (b.x < a.x - kEps) {
            return b;
        }
        return (a.y <= b.y) ? a : b;
    }

    [[nodiscard]] static bool PointOnSegment(const Point2D &p, const Line &line) noexcept {
        const Point2D ab = line.end - line.start;
        const Point2D ap = p - line.start;

        if (SamePoint(line.start, line.end)) {
            return SamePoint(p, line.start);
        }

        if (!IsZero(ab.Cross(ap))) {
            return false;
        }

        const double dot = ap.Dot(ab);
        const double len2 = ab.Dot(ab);
        return dot >= -kEps && dot <= len2 + kEps;
    }

    [[nodiscard]] static std::optional<Point2D> IntersectLineLine(const Line &lhs, const Line &rhs) {
        if (SamePoint(lhs.start, lhs.end) && SamePoint(rhs.start, rhs.end)) {
            return SamePoint(lhs.start, rhs.start) ? std::optional<Point2D>{lhs.start} : std::nullopt;
        }

        if (SamePoint(lhs.start, lhs.end)) {
            return PointOnSegment(lhs.start, rhs) ? std::optional<Point2D>{lhs.start} : std::nullopt;
        }

        if (SamePoint(rhs.start, rhs.end)) {
            return PointOnSegment(rhs.start, lhs) ? std::optional<Point2D>{rhs.start} : std::nullopt;
        }

        const Point2D p = lhs.start;
        const Point2D r = lhs.end - lhs.start;
        const Point2D q = rhs.start;
        const Point2D s = rhs.end - rhs.start;

        const double rxs = r.Cross(s);
        const Point2D qp = q - p;
        const double qpxr = qp.Cross(r);

        if (IsZero(rxs) && IsZero(qpxr)) {
            std::optional<Point2D> result;

            auto try_update = [&](const Point2D &candidate) {
                if (PointOnSegment(candidate, lhs) && PointOnSegment(candidate, rhs)) {
                    if (result.has_value()) {
                        result = LexicographicallyMin(*result, candidate);
                    } else {
                        result = candidate;
                    }
                }
            };

            try_update(lhs.start);
            try_update(lhs.end);
            try_update(rhs.start);
            try_update(rhs.end);

            return result;
        }

        if (IsZero(rxs)) {
            return std::nullopt;
        }

        const double t = qp.Cross(s) / rxs;
        const double u = qp.Cross(r) / rxs;

        if (IsInUnitSegment(t) && IsInUnitSegment(u)) {
            return p + r * t;
        }

        return std::nullopt;
    }

    [[nodiscard]] static std::optional<Point2D> IntersectLineCircle(const Line &line, const Circle &circle) {
        const Point2D d = line.end - line.start;
        const Point2D f = line.start - circle.center_p;

        const double a = d.Dot(d);
        const double b = 2.0 * f.Dot(d);
        const double c = f.Dot(f) - circle.radius * circle.radius;

        if (IsZero(a)) {
            return IsZero(c) ? std::optional<Point2D>{line.start} : std::nullopt;
        }

        const double discriminant = b * b - 4.0 * a * c;

        if (discriminant < -kEps) {
            return std::nullopt;
        }

        if (IsZero(discriminant)) {
            const double t = -b / (2.0 * a);
            return IsInUnitSegment(t) ? std::optional<Point2D>{line.start + d * t} : std::nullopt;
        }

        const double sqrt_d = std::sqrt(std::max(0.0, discriminant));
        const double t1 = (-b - sqrt_d) / (2.0 * a);
        const double t2 = (-b + sqrt_d) / (2.0 * a);

        if (IsInUnitSegment(t1)) {
            return line.start + d * t1;
        }
        if (IsInUnitSegment(t2)) {
            return line.start + d * t2;
        }

        return std::nullopt;
    }

    [[nodiscard]] static std::optional<Point2D> IntersectCircleCircle(const Circle &lhs, const Circle &rhs) {
        const Point2D delta = rhs.center_p - lhs.center_p;
        const double d = delta.Length();

        if (IsZero(d) && IsZero(lhs.radius - rhs.radius)) {
            return std::nullopt;
        }

        if (d > lhs.radius + rhs.radius + kEps) {
            return std::nullopt;
        }

        if (d < std::abs(lhs.radius - rhs.radius) - kEps) {
            return std::nullopt;
        }

        if (IsZero(d)) {
            return std::nullopt;
        }

        const double a = (lhs.radius * lhs.radius - rhs.radius * rhs.radius + d * d) / (2.0 * d);
        const double h2 = lhs.radius * lhs.radius - a * a;

        if (h2 < -kEps) {
            return std::nullopt;
        }

        const Point2D base = lhs.center_p + delta * (a / d);

        if (IsZero(h2)) {
            return base;
        }

        const double h = std::sqrt(std::max(0.0, h2));
        const Point2D offset{-delta.y * (h / d), delta.x * (h / d)};

        const Point2D p1 = base + offset;
        const Point2D p2 = base - offset;

        return LexicographicallyMin(p1, p2);
    }
};

inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections
