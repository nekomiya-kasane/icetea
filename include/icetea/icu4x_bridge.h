#pragma once

/// @file icu4x_bridge.h
/// @brief C++ wrapper around ICU4X Rust library for locale-aware formatting.
///
/// Provides plural rules, number/date/time/list/currency/relative-time formatting
/// via the ICU4X C FFI. Locale can be changed at runtime.

#include "icetea/exports.h"
#include "icetea/types.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace icetea {

    /// Maximum supported locale tag length (BCP-47).
    inline constexpr size_t max_locale_length = 35;

    /// @brief C++ wrapper around ICU4X Rust library.
    ///
    /// Wraps the opaque Icu4xBridge handle and provides locale-aware formatting.
    /// Not thread-safe for mutation (set_locale); read operations are safe if the
    /// locale is not being changed concurrently.
    class ICETEA_API icu4x_bridge {
      public:
        icu4x_bridge() = default;
        ~icu4x_bridge();

        icu4x_bridge(icu4x_bridge &&other) noexcept;
        icu4x_bridge &operator=(icu4x_bridge &&other) noexcept;

        icu4x_bridge(const icu4x_bridge &) = delete;
        icu4x_bridge &operator=(const icu4x_bridge &) = delete;

        // ── Nested type aliases (for backward compatibility) ─────────────────

        using date_style = icetea::date_style;
        using list_type = icetea::list_type;
        using reltime_unit = icetea::reltime_unit;

        // ── Locale management ────────────────────────────────────────────────

        /// Set the active locale.
        /// With lazy=true (default), the bridge is created on first formatting call.
        /// With lazy=false, the bridge is created immediately (original behavior).
        [[nodiscard]] auto set_locale(std::string_view locale, bool lazy = true) -> result<void>;

        /// Get the current locale tag.
        [[nodiscard]] auto current_locale() const -> std::string;

        /// Check if the bridge is initialized (locale set successfully).
        [[nodiscard]] auto is_initialized() const noexcept -> bool;

        // ── Plural rules ─────────────────────────────────────────────────────

        /// Get the cardinal plural category for a number.
        [[nodiscard]] auto cardinal_category(double n) const -> plural_category;

        /// Get the ordinal plural category for a number.
        [[nodiscard]] auto ordinal_category(double n) const -> plural_category;

        /// Get the cardinal plural category for a range [start, end].
        [[nodiscard]] auto cardinal_range(double start, double end) const -> plural_category;

        // ── Number formatting ────────────────────────────────────────────────

        /// Format a number using the locale's decimal formatter.
        /// Appends the formatted result to `out`.
        void format_number(std::string &out, double n, int fraction_digits = -1) const;

        // ── Date/Time formatting ─────────────────────────────────────────────

        /// Format a date. Appends to `out`.
        void format_date(std::string &out, int year, int month, int day,
                         date_style style = date_style::medium_fmt) const;

        /// Format a time. Appends to `out`.
        void format_time(std::string &out, int hour, int minute, int second) const;

        // ── List formatting ──────────────────────────────────────────────────

        /// Format a list of strings. Appends to `out`.
        void format_list(std::string &out, std::span<const std::string_view> items,
                         list_type type = list_type::conjunction) const;

        // ── Currency formatting ──────────────────────────────────────────────

        /// Format a currency amount. Appends to `out`.
        void format_currency(std::string &out, double amount, std::string_view currency_code) const;

        // ── Relative time formatting ─────────────────────────────────────────

        /// Format a relative time value. Appends to `out`.
        void format_reltime(std::string &out, double value, reltime_unit unit) const;

      private:
        /// @brief Ensure the ICU4X bridge is initialized (lazy creation on first use).
        [[nodiscard]] auto ensure_initialized() const -> result<void>;

        struct opaque_deleter {
            void operator()(void *p) const noexcept;
        };
        mutable std::unique_ptr<void, opaque_deleter> handle_;
        mutable std::string locale_tag_; // stored for deferred bridge creation
    };

} // namespace icetea
