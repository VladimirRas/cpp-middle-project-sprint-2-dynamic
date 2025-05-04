#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <charconv>
#include <tuple>
#include <type_traits>

#include "types.hpp"

namespace stdx::details {

template <typename T>
constexpr bool is_supported_type_v = 
    std::is_same_v<std::remove_cv_t<T>, int8_t> ||
    std::is_same_v<std::remove_cv_t<T>, int16_t> ||
    std::is_same_v<std::remove_cv_t<T>, int32_t> ||
    std::is_same_v<std::remove_cv_t<T>, int64_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint8_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint16_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint32_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint64_t> ||
    std::is_same_v<std::remove_cv_t<T>, float> ||
    std::is_same_v<std::remove_cv_t<T>, double> ||
    std::is_same_v<std::remove_cv_t<T>, std::string> ||
    std::is_same_v<std::remove_cv_t<T>, std::string_view>;

template <typename T>
concept SupportedType = is_supported_type_v<T>;

template <SupportedType T>
std::expected<T, scan_error> parse_value(std::string_view input) {
    if constexpr (std::is_same_v<std::remove_cv_t<T>, std::string> || 
                    std::is_same_v<std::remove_cv_t<T>, std::string_view>) {
        return T(input);
    } else {
        T value;
        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);
        if (ec != std::errc()) {
            return std::unexpected(scan_error{"Fail to parse value"});
        }
        return value;
    }
}

// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    if (fmt.empty()) {
        return parse_value<T>(input);
    }

    if (fmt[0] != '%') {
        return std::unexpected(scan_error{"Invalid format specifier"});
    }

    using BaseType = std::decay_t<T>;

    char spec = fmt.size() > 1 ? fmt[1] : '\0';
    
    if constexpr (std::is_integral_v<BaseType>) {
        if (std::is_signed_v<BaseType> && spec != 'd') {
            return std::unexpected(scan_error{"Type or specifier mismatch: expected %d"});
        }
        if (!std::is_signed_v<BaseType> && spec != 'u') {
            return std::unexpected(scan_error{"Type or specifier mismatch: expected %u"});
        }
    } else if constexpr (std::is_floating_point_v<BaseType>) {
        if (spec != 'f') {
            return std::unexpected(scan_error{"Type/specifier mismatch: expected %f"});
        }
    } else if constexpr (std::is_same_v<BaseType, std::string> || 
                         std::is_same_v<BaseType, std::string_view>) {
        if (spec != 's') {
            return std::unexpected(scan_error{"Type/specifier mismatch: expected %s"});
        }
    }

    return parse_value<T>(input);
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

} // namespace stdx::details