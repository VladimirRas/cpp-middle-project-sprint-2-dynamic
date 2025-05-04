#pragma once

#include "parse.hpp"
#include "types.hpp"

namespace stdx {

template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> 
scan(std::string_view input, std::string_view format) {
    static_assert((details::is_supported_type_v<Ts> && ...),
        "All types must be supported by scan function");

    auto parsed = details::parse_sources<Ts...>(input, format);
    if (!parsed) {
        return std::unexpected(parsed.error());
    }

    auto [format_parts, input_parts] = *parsed;
    if (format_parts.size() != sizeof...(Ts)) {
        return std::unexpected(details::scan_error{
            "Number of placeholders not match number of types"});
    }

    std::tuple<std::expected<Ts, details::scan_error>...> results;
    
    auto process_results = [&]<size_t... I>(std::index_sequence<I...>) {
        ((std::get<I>(results) = details::parse_value_with_format<Ts>(
            input_parts[I], format_parts[I])), ...);

        bool has_errors = (... || !std::get<I>(results).has_value());
        
        if (has_errors) {
            std::string error_msg;
            ((!std::get<I>(results) ? 
                (error_msg += std::get<I>(results).error().message + "; ") : void()), ...);
            
            return std::unexpected(details::scan_error{std::move(error_msg)});
        }
        
        return std::expected<details::scan_result<Ts...>, details::scan_error>(
            details::scan_result<Ts...>{std::tuple{
                std::get<I>(results).value()...
            }}
        );
    };

    return process_results(std::index_sequence_for<Ts...>{});
}

} // namespace stdx
