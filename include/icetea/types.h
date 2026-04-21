#pragma once

/// @file types.h
/// @brief Core types shared across icetea and its consumers.

#include "icetea/exports.h"

#include <cstdint>
#include <expected>
#include <string>

namespace icetea {

    // ── Plural categories (CLDR) ──────────────────────────────────────────────

    enum class plural_category : uint8_t { zero = 0, one = 1, two = 2, few = 3, many = 4, other = 5 };

    // ── Error types ───────────────────────────────────────────────────────────

    enum class error_code : uint8_t {
        ok = 0,
        icu4x_init_failed,
        locale_not_found,
        invalid_argument,
        buffer_too_small,
    };

    struct error {
        error_code code = error_code::ok;
        std::string message;
    };

    template <typename T> using result = std::expected<T, error>;

    // ── Date/Time/List formatting enums ───────────────────────────────────────

    enum class date_style : int { short_fmt = 0, medium_fmt = 1, long_fmt = 2 };
    enum class list_type : int { conjunction = 0, disjunction = 1, unit = 2 };
    enum class reltime_unit : int { second = 0, minute = 1, hour = 2, day = 3, week = 4, month = 5, year = 6 };

} // namespace icetea
