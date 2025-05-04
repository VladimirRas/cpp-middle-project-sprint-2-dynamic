#pragma once

#include <expected>
#include <string>
#include <tuple>
#include <type_traits>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> values;

    template <size_t I>
    auto get() const -> std::tuple_element_t<I, std::tuple<Ts...>> {
        return std::get<I>(values);
    }

    template <typename T>
    auto get() const -> T {
        return std::get<T>(values);
    }
};

} // namespace stdx::details
