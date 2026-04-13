#include "shape_utils.hpp"
#include <charconv>
#include <functional>

namespace geometry::utils {
// Разбивает строку на слова (по пробелам), игнорируя лишние пробелы
std::vector<std::string_view> SplitIntoWords(std::string_view s) {
    std::vector<std::string_view> words;
    size_t start = 0;
    size_t end = 0;

    while (start < s.size()) {
        // Пропускаем пробелы
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;
        if (start >= s.size())
            break;
        end = start;
        while (end < s.size() && !std::isspace(static_cast<unsigned char>(s[end])))
            ++end;
        words.push_back(s.substr(start, end - start));
        start = end;
    }
    return words;
}

// Безопасный парсинг строки в double (без исключений)
std::optional<double> ParseDouble(std::string_view s) {
    double value = 0.0;
    auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
        return value;
    }
    return std::nullopt;
}

// Парсит строку в вектор double
std::optional<std::vector<double>> ParseDoubles(std::string_view s) {
    if (s.empty())
        return std::nullopt;
    auto tokens = SplitIntoWords(s);
    if (tokens.empty())
        return std::nullopt;

    std::vector<double> result;
    result.reserve(tokens.size());

    for (auto token : tokens) {
        auto num = ParseDouble(token);
        if (!num.has_value()) {
            return std::nullopt;
        }
        result.push_back(*num);
    }
    return result;
}

// Проверяет размер вектора и возвращает его, если совпадает
std::optional<std::vector<double>> RequireSize(const std::vector<double> &v, size_t expected) {
    return (v.size() == expected) ? std::make_optional(v) : std::nullopt;
}

// Проверяет, что значение > 0
std::optional<double> RequirePositive(double x) { return (x > 0) ? std::make_optional(x) : std::nullopt; }

// Проверяет, что double представляет целое число >= min_value
std::optional<int> RequireIntegerAtLeast(double d, int min_value) {
    int i = static_cast<int>(d);
    if (static_cast<double>(i) == d && i >= min_value) {
        return i;
    }
    return std::nullopt;
}

// Конструкторы фигур

/**
    @brief Создаёт круг из параметров
    @note Пример того как могла бы выглядеть эта функция:
        if (v.size() != 3) return std::nullopt;
        if (v[2] <= 0) return std::nullopt; // радиус должен быть > 0
        return Circle{{v[0], v[1]}, v[2]};
*/
std::optional<Shape> MakeCircle(const std::vector<double> &v) {
    return RequireSize(v, 3).and_then([](const std::vector<double> &values) {
        return RequirePositive(values[2]).and_then(
            [&](double radius) -> std::optional<Shape> { return Shape{Circle{{values[0], values[1]}, radius}}; });
    });
}

std::optional<Shape> MakeLine(const std::vector<double> &v) {
    return RequireSize(v, 4).and_then([](const std::vector<double> &values) -> std::optional<Shape> {
        return Shape{Line{{values[0], values[1]}, {values[2], values[3]}}};
    });
}

std::optional<Shape> MakeTriangle(const std::vector<double> &v) {
    return RequireSize(v, 6).and_then([](const std::vector<double> &values) -> std::optional<Shape> {
        return Shape{Triangle{{values[0], values[1]}, {values[2], values[3]}, {values[4], values[5]}}};
    });
}

std::optional<Shape> MakeRectangle(const std::vector<double> &v) {
    return RequireSize(v, 4).and_then([](const std::vector<double> &values) {
        return RequirePositive(values[2]).and_then([&](double width) {
            return RequirePositive(values[3]).and_then([&](double height) -> std::optional<Shape> {
                return Shape{Rectangle{{values[0], values[1]}, width, height}};
            });
        });
    });
}

std::optional<Shape> MakePolygon(const std::vector<double> &v) {
    return RequireSize(v, 4).and_then([](const std::vector<double> &values) {
        return RequirePositive(values[2]).and_then([&](double radius) {
            return RequireIntegerAtLeast(values[3], 3).and_then([&](int sides) -> std::optional<Shape> {
                return Shape{RegularPolygon{{values[0], values[1]}, radius, sides}};
            });
        });
    });
}

// Парсинг одной фигуры
std::optional<Shape> ParseSingleShape(std::string_view token) {
    auto parts = SplitIntoWords(token);
    if (parts.empty())
        return std::nullopt;

    std::string_view type = parts[0];
    std::string param_str;
    for (auto i : std::views::iota(1u, parts.size())) {
        if (!param_str.empty())
            param_str += ' ';
        param_str += std::string(parts[i]);
    }

    // Выбираем конструктор по имени
    auto get_maker =
        [](std::string_view t) -> std::optional<std::function<std::optional<Shape>(const std::vector<double> &)>> {
        if (t == "circle")
            return MakeCircle;
        if (t == "line")
            return MakeLine;
        if (t == "triangle")
            return MakeTriangle;
        if (t == "rectangle")
            return MakeRectangle;
        if (t == "polygon")
            return MakePolygon;
        return std::nullopt;
    };

    // Обратите внимание на код ниже
    return get_maker(type).and_then([&](auto maker) { return ParseDoubles(param_str).and_then(maker); });
}

std::vector<Shape> ParseShapes(std::string_view input) {
    std::vector<Shape> result;

    // Разделяем по ';'
    size_t start = 0;
    size_t end = 0;
    while (start < input.size()) {
        end = input.find(';', start);
        if (end == std::string_view::npos)
            end = input.size();

        std::string_view token = input.substr(start, end - start);
        // Убираем пробелы по краям
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front())))
            token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))
            token.remove_suffix(1);

        if (!token.empty()) {
            auto shape_opt = ParseSingleShape(token);
            if (shape_opt.has_value()) {
                result.push_back(*shape_opt);
            }
        }

        start = end + 1;
    }

    return result;
}

std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes) {
    std::vector<std::pair<Shape, Shape>> collisions;

    const auto get_bounding_box = [](const Shape &shape) {
        return std::visit([](const auto &s) { return s.BoundBox(); }, shape);
    };

    for (size_t i : std::views::iota(size_t{0}, shapes.size())) {
        for (size_t j : std::views::iota(i + 1, shapes.size())) {
            if (get_bounding_box(shapes[i]).Overlaps(get_bounding_box(shapes[j]))) {
                collisions.emplace_back(shapes[i], shapes[j]);
            }
        }
    }

    return collisions;
}

std::optional<size_t> FindHighestShape(std::span<const Shape> shapes) {
    if (shapes.empty()) {
        return std::nullopt;
    }

    const auto get_height = [&](size_t index) {
        return std::visit([](const auto &s) { return s.Height(); }, shapes[index]);
    };

    const auto indices = std::views::iota(size_t{0}, shapes.size());
    const auto it = std::ranges::max_element(indices, std::less{}, get_height);

    return *it;
}
}  // namespace geometry::utils
