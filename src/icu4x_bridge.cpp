#include "icetea/icu4x_bridge.h"

#include "icetea/icu4x_capi.h"

#include <array>
#include <vector>

namespace icetea {

    void icu4x_bridge::opaque_deleter::operator()(void *p) const noexcept {
        if (p) {
            icu4x_bridge_destroy(static_cast<Icu4xBridge *>(p));
        }
    }

    icu4x_bridge::~icu4x_bridge() = default;

    icu4x_bridge::icu4x_bridge(icu4x_bridge &&other) noexcept = default;
    icu4x_bridge &icu4x_bridge::operator=(icu4x_bridge &&other) noexcept = default;

    auto icu4x_bridge::set_locale(std::string_view locale, bool lazy) -> result<void> {
        if (locale.size() > max_locale_length) {
            return std::unexpected(error{error_code::invalid_argument, std::string("ICU4X: locale tag too long (") +
                                                                           std::to_string(locale.size()) + " > " +
                                                                           std::to_string(max_locale_length) + ")"});
        }
        handle_.reset(); // release any previous bridge
        locale_tag_.assign(locale);

        if (!lazy) {
            return ensure_initialized();
        }
        return {};
    }

    auto icu4x_bridge::ensure_initialized() const -> result<void> {
        if (handle_) {
            return {}; // already created
        }
        if (locale_tag_.empty()) {
            return std::unexpected(error{error_code::invalid_argument, "ICU4X: no locale set"});
        }
        auto *raw = icu4x_bridge_create(locale_tag_.c_str());
        if (!raw) {
            return std::unexpected(
                error{error_code::icu4x_init_failed,
                      std::string("ICU4X: failed to create bridge for locale '") + locale_tag_ + "'"});
        }
        handle_.reset(raw);
        return {};
    }

    auto icu4x_bridge::current_locale() const -> std::string {
        if (handle_) {
            std::array<char, 64> buf{};
            auto n = icu4x_get_locale(static_cast<const Icu4xBridge *>(handle_.get()), buf.data(),
                                      static_cast<int>(buf.size()));
            if (n > 0) {
                return std::string(buf.data(), static_cast<size_t>(n));
            }
        }
        return locale_tag_; // return stored tag even before bridge creation
    }

    auto icu4x_bridge::is_initialized() const noexcept -> bool {
        return handle_ != nullptr || !locale_tag_.empty();
    }

    auto icu4x_bridge::cardinal_category(double n) const -> plural_category {
        if (auto r = ensure_initialized(); !r) {
            return plural_category::other;
        }
        auto cat = icu4x_cardinal_category(static_cast<const Icu4xBridge *>(handle_.get()), n);
        if (cat < 0 || cat > 5) {
            return plural_category::other;
        }
        return static_cast<plural_category>(cat);
    }

    auto icu4x_bridge::ordinal_category(double n) const -> plural_category {
        if (auto r = ensure_initialized(); !r) {
            return plural_category::other;
        }
        auto cat = icu4x_ordinal_category(static_cast<const Icu4xBridge *>(handle_.get()), n);
        if (cat < 0 || cat > 5) {
            return plural_category::other;
        }
        return static_cast<plural_category>(cat);
    }

    void icu4x_bridge::format_number(std::string &out, double n, int fraction_digits) const {
        if (auto r = ensure_initialized(); !r) {
            out.append(std::to_string(n));
            return;
        }
        std::array<char, 128> buf{};
        auto written = icu4x_format_number(static_cast<const Icu4xBridge *>(handle_.get()), n, fraction_digits,
                                           buf.data(), static_cast<int>(buf.size()));
        if (written < 0) {
            out.append(std::to_string(n));
            return;
        }
        if (written >= static_cast<int>(buf.size())) {
            std::string big(static_cast<size_t>(written) + 1, '\0');
            icu4x_format_number(static_cast<const Icu4xBridge *>(handle_.get()), n, fraction_digits, big.data(),
                                static_cast<int>(big.size()));
            out.append(big.data(), static_cast<size_t>(written));
            return;
        }
        out.append(buf.data(), static_cast<size_t>(written));
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Helper for FFI buffer calls
    // ═══════════════════════════════════════════════════════════════════════

    namespace {

        /// Call an FFI function that writes into a char buffer.
        /// Appends the result to `out`. Falls back to `fallback` on error.
        template <typename Fn>
        void ffi_format_into(std::string &out, std::string_view fallback, const void *handle, Fn &&fn) {
            if (!handle) {
                out.append(fallback);
                return;
            }
            std::array<char, 256> buf{};
            auto written = fn(static_cast<const Icu4xBridge *>(handle), buf.data(), static_cast<int>(buf.size()));
            if (written < 0) {
                out.append(fallback);
                return;
            }
            if (written >= static_cast<int>(buf.size())) {
                std::string big(static_cast<size_t>(written) + 1, '\0');
                fn(static_cast<const Icu4xBridge *>(handle), big.data(), static_cast<int>(big.size()));
                out.append(big.data(), static_cast<size_t>(written));
                return;
            }
            out.append(buf.data(), static_cast<size_t>(written));
        }

    } // namespace

    // ═══════════════════════════════════════════════════════════════════════
    //  Date/Time formatting
    // ═══════════════════════════════════════════════════════════════════════

    void icu4x_bridge::format_date(std::string &out, int year, int month, int day, date_style style) const {
        ffi_format_into(out, std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day),
                        handle_.get(), [&](const Icu4xBridge *b, char *buf, int len) {
                            return icu4x_format_date(b, year, month, day, static_cast<int>(style), buf, len);
                        });
    }

    void icu4x_bridge::format_time(std::string &out, int hour, int minute, int second) const {
        auto fallback = std::to_string(hour) + ":" + (minute < 10 ? "0" : "") + std::to_string(minute) + ":" +
                        (second < 10 ? "0" : "") + std::to_string(second);
        ffi_format_into(out, fallback, handle_.get(), [&](const Icu4xBridge *b, char *buf, int len) {
            return icu4x_format_time(b, hour, minute, second, buf, len);
        });
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  List formatting
    // ═══════════════════════════════════════════════════════════════════════

    void icu4x_bridge::format_list(std::string &out, std::span<const std::string_view> items, list_type type) const {
        if (items.empty()) {
            return;
        }

        if (!handle_) {
            // Fallback: comma-separated
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) {
                    out.append(", ");
                }
                out.append(items[i]);
            }
            return;
        }

        // Build null-terminated C strings + pointer array
        std::vector<std::string> owned;
        owned.reserve(items.size());
        for (auto sv : items) {
            owned.emplace_back(sv);
        }
        std::vector<const char *> c_ptrs;
        c_ptrs.reserve(items.size());
        for (const auto &s : owned) {
            c_ptrs.push_back(s.c_str());
        }

        std::array<char, 512> buf{};
        auto written = icu4x_format_list(static_cast<const Icu4xBridge *>(handle_.get()), c_ptrs.data(),
                                         static_cast<int>(c_ptrs.size()), static_cast<int>(type), buf.data(),
                                         static_cast<int>(buf.size()));

        if (written < 0) {
            // Fallback
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) {
                    out.append(", ");
                }
                out.append(items[i]);
            }
            return;
        }
        if (written >= static_cast<int>(buf.size())) {
            std::string big(static_cast<size_t>(written) + 1, '\0');
            icu4x_format_list(static_cast<const Icu4xBridge *>(handle_.get()), c_ptrs.data(),
                              static_cast<int>(c_ptrs.size()), static_cast<int>(type), big.data(),
                              static_cast<int>(big.size()));
            out.append(big.data(), static_cast<size_t>(written));
            return;
        }
        out.append(buf.data(), static_cast<size_t>(written));
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Currency formatting
    // ═══════════════════════════════════════════════════════════════════════

    void icu4x_bridge::format_currency(std::string &out, double amount, std::string_view currency_code) const {
        std::string code_str(currency_code);
        ffi_format_into(out, std::to_string(amount) + " " + code_str, handle_.get(),
                        [&](const Icu4xBridge *b, char *buf, int len) {
                            return icu4x_format_currency(b, amount, code_str.c_str(), buf, len);
                        });
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Relative time formatting
    // ═══════════════════════════════════════════════════════════════════════

    void icu4x_bridge::format_reltime(std::string &out, double value, reltime_unit unit) const {
        auto iv = static_cast<int>(value);
        auto fallback = iv < 0 ? std::to_string(-iv) + " units ago" : "in " + std::to_string(iv) + " units";
        ffi_format_into(out, fallback, handle_.get(), [&](const Icu4xBridge *b, char *buf, int len) {
            return icu4x_format_reltime(b, value, static_cast<int>(unit), buf, len);
        });
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Cardinal range plural
    // ═══════════════════════════════════════════════════════════════════════

    auto icu4x_bridge::cardinal_range(double start, double end) const -> plural_category {
        if (auto r = ensure_initialized(); !r) {
            return plural_category::other;
        }
        auto cat = icu4x_cardinal_range(static_cast<const Icu4xBridge *>(handle_.get()), start, end);
        if (cat < 0 || cat > 5) {
            return plural_category::other;
        }
        return static_cast<plural_category>(cat);
    }

} // namespace icetea
