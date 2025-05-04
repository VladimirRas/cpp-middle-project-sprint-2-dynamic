#include <gtest/gtest.h>
#include <print>

#include "scan.hpp"

TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");
    ASSERT_FALSE(result);
}

TEST(ScanTest, BasicIntegerParsing) {
    constexpr auto result = stdx::scan<"{} {} {}"_fmt, int, int, int>("20 10 35");
    static_assert(result.has_value());
    static_assert(result->get<0>() == 20);
    static_assert(result->get<1>() == 10);
    static_assert(result->get<2>() == 35);
}

TEST(ScanTest, DifferentNumberTypes) {
    constexpr auto result = stdx::scan<"{} {} {}"_fmt, int8_t, uint64_t, double>("-5 18446744073709551615 3.14");
    static_assert(result.has_value());
    static_assert(result->get<0>() == -5);
    static_assert(result->get<1>() == 18446744073709551615ULL);
    static_assert(result->get<2>() == 3.14);
}

TEST(ScanTest, StringParsing) {
    constexpr auto result = stdx::scan<"{} {}"_fmt, std::string_view, std::string_view>("Hi Cpp");
    static_assert(result.has_value());
    static_assert(result->get<0>() == "Hi");
    static_assert(result->get<1>() == "Cpp");
}

TEST(ScanTest, FormatSpecifiers) {
    constexpr auto result = stdx::scan<"{%d} {%u} {%s}"_fmt, int, unsigned, std::string_view>("-52 100 Hello");
    static_assert(result.has_value());
    static_assert(result->get<0>() == -52);
    static_assert(result->get<1>() == 100);
    static_assert(result->get<2>() == "Hello");
}

TEST(ScanTest, TypeSpecifierMismatch) {
    constexpr auto result1 = stdx::scan<"{%u}"_fmt, int>("44");
    static_assert(!result1.has_value());
    
    constexpr auto result2 = stdx::scan<"{%d}"_fmt, unsigned>("44");
    static_assert(!result2.has_value());
    
    constexpr auto result3 = stdx::scan<"{%s}"_fmt, int>("44");
    static_assert(!result3.has_value());
}

TEST(ScanTest, CVQualifiedTypes) {
    constexpr auto result = stdx::scan<"{} {} {}"_fmt, const int, volatile uint32_t, const volatile int8_t>("10 25 -5");
    static_assert(result.has_value());
    static_assert(result->get<0>() == 10);
    static_assert(result->get<1>() == 25);
    static_assert(result->get<2>() == -5);
}

TEST(ScanTest, InvalidInput) {
    constexpr auto result1 = stdx::scan<"{}"_fmt, int>("not_a_number");
    static_assert(!result1.has_value());
    
    constexpr auto result2 = stdx::scan<"{}"_fmt, double>("3.14.15");
    static_assert(!result2.has_value());
}

TEST(ScanTest, PlaceholderCountMismatch) {
    constexpr bool compiles = requires {
        stdx::scan<"{} {}"_fmt, int>("10");
    };
    static_assert(!compiles);
}

TEST(ScanTest, ComplexFormatString) {
    constexpr auto result = stdx::scan<"Value: {%d}, Count: {%u}, Name: {%s}"_fmt, 
                                     int, unsigned, std::string_view>(
        "Value: -44, Count: 10, Name: TesT");
    static_assert(result.has_value());
    static_assert(result->get<0>() == -44);
    static_assert(result->get<1>() == 10);
    static_assert(result->get<2>() == "TesT");
}

TEST(ScanTest, EmptyPlaceholders) {
    constexpr auto result = stdx::scan<"{} {}"_fmt, std::string_view, std::string_view>("  ");
    static_assert(result.has_value());
    static_assert(result->get<0>().empty());
    static_assert(result->get<1>().empty());
}

TEST(ScanTest, BoundaryValues) {
    constexpr auto result = stdx::scan<"{} {} {}"_fmt, 
                                     int8_t, uint8_t, int64_t>(
        "-128 255 9223372036854775807");
    static_assert(result.has_value());
    static_assert(result->get<0>() == -128);
    static_assert(result->get<1>() == 255);
    static_assert(result->get<2>() == 9223372036854775807LL);
}

TEST(ScanTest, FormatMismatch) {
    constexpr auto result = stdx::scan<"Hello {}"_fmt, int>("World 42");
    static_assert(!result.has_value());
}

TEST(ScanTest, PartialMatch) {
    constexpr auto result = stdx::scan<"{} {} {}"_fmt, int, int, int>("10 20");
    static_assert(!result.has_value());
}