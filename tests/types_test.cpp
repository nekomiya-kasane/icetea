#include "icetea/types.h"

#include <cstdint>
#include <expected>
#include <gtest/gtest.h>
#include <string>

using namespace icetea;

// ── plural_category enum ────────────────────────────────────────────────

TEST(PluralCategory, Values) {
    EXPECT_EQ(static_cast<uint8_t>(plural_category::zero), 0);
    EXPECT_EQ(static_cast<uint8_t>(plural_category::one), 1);
    EXPECT_EQ(static_cast<uint8_t>(plural_category::two), 2);
    EXPECT_EQ(static_cast<uint8_t>(plural_category::few), 3);
    EXPECT_EQ(static_cast<uint8_t>(plural_category::many), 4);
    EXPECT_EQ(static_cast<uint8_t>(plural_category::other), 5);
}

// ── error_code enum ─────────────────────────────────────────────────────

TEST(ErrorCode, Values) {
    EXPECT_EQ(static_cast<uint8_t>(error_code::ok), 0);
    EXPECT_EQ(static_cast<uint8_t>(error_code::icu4x_init_failed), 1);
    EXPECT_EQ(static_cast<uint8_t>(error_code::locale_not_found), 2);
    EXPECT_EQ(static_cast<uint8_t>(error_code::invalid_argument), 3);
    EXPECT_EQ(static_cast<uint8_t>(error_code::buffer_too_small), 4);
}

// ── error struct ────────────────────────────────────────────────────────

TEST(Error, DefaultIsOk) {
    error e;
    EXPECT_EQ(e.code, error_code::ok);
    EXPECT_TRUE(e.message.empty());
}

TEST(Error, WithMessage) {
    error e{error_code::locale_not_found, "en-XX not available"};
    EXPECT_EQ(e.code, error_code::locale_not_found);
    EXPECT_EQ(e.message, "en-XX not available");
}

// ── result<T> alias ─────────────────────────────────────────────────────

TEST(Result, SuccessInt) {
    result<int> r = 42;
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(*r, 42);
}

TEST(Result, SuccessString) {
    result<std::string> r = std::string("hello");
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(*r, "hello");
}

TEST(Result, ErrorCase) {
    result<int> r = std::unexpected(error{error_code::invalid_argument, "bad input"});
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error().code, error_code::invalid_argument);
    EXPECT_EQ(r.error().message, "bad input");
}

TEST(Result, VoidSuccess) {
    result<void> r{};
    EXPECT_TRUE(r.has_value());
}

TEST(Result, VoidError) {
    result<void> r = std::unexpected(error{error_code::icu4x_init_failed, "init failed"});
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error().code, error_code::icu4x_init_failed);
}

// ── date_style enum ─────────────────────────────────────────────────────

TEST(DateStyle, Values) {
    EXPECT_EQ(static_cast<int>(date_style::short_fmt), 0);
    EXPECT_EQ(static_cast<int>(date_style::medium_fmt), 1);
    EXPECT_EQ(static_cast<int>(date_style::long_fmt), 2);
}

// ── list_type enum ──────────────────────────────────────────────────────

TEST(ListType, Values) {
    EXPECT_EQ(static_cast<int>(list_type::conjunction), 0);
    EXPECT_EQ(static_cast<int>(list_type::disjunction), 1);
    EXPECT_EQ(static_cast<int>(list_type::unit), 2);
}

// ── reltime_unit enum ───────────────────────────────────────────────────

TEST(ReltimeUnit, Values) {
    EXPECT_EQ(static_cast<int>(reltime_unit::second), 0);
    EXPECT_EQ(static_cast<int>(reltime_unit::minute), 1);
    EXPECT_EQ(static_cast<int>(reltime_unit::hour), 2);
    EXPECT_EQ(static_cast<int>(reltime_unit::day), 3);
    EXPECT_EQ(static_cast<int>(reltime_unit::week), 4);
    EXPECT_EQ(static_cast<int>(reltime_unit::month), 5);
    EXPECT_EQ(static_cast<int>(reltime_unit::year), 6);
}
