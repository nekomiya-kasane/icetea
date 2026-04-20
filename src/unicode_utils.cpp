#include "icetea/unicode_utils.h"

#include "icetea/icu4x_capi.h"

#include <array>
#include <string>

namespace icetea {

// ── Internal helper ──────────────────────────────────────────────────

namespace {

/// Call an FFI function that writes into a fixed-size buffer,
/// retrying with a larger buffer if needed.
template <typename Fn> std::string ffi_string_call(Fn &&fn) {
    std::array<char, 512> buf{};
    int written = fn(buf.data(), static_cast<int>(buf.size()));
    if (written < 0) return {};
    if (written >= static_cast<int>(buf.size())) {
        std::string big(static_cast<size_t>(written) + 1, '\0');
        fn(big.data(), static_cast<int>(big.size()));
        return std::string(big.data(), static_cast<size_t>(written));
    }
    return std::string(buf.data(), static_cast<size_t>(written));
}

} // namespace

// ── Case Mapping ─────────────────────────────────────────────────────

std::string lowercase(std::string_view src, std::string_view locale) {
    std::string locale_z(locale);
    return ffi_string_call([&](char *buf, int len) {
        return icu4x_lowercase(src.data(), static_cast<int>(src.size()), locale_z.c_str(), buf, len);
    });
}

std::string uppercase(std::string_view src, std::string_view locale) {
    std::string locale_z(locale);
    return ffi_string_call([&](char *buf, int len) {
        return icu4x_uppercase(src.data(), static_cast<int>(src.size()), locale_z.c_str(), buf, len);
    });
}

std::string case_fold(std::string_view src) {
    return ffi_string_call(
        [&](char *buf, int len) { return icu4x_case_fold(src.data(), static_cast<int>(src.size()), buf, len); });
}

std::string titlecase_segment(std::string_view src, std::string_view locale) {
    std::string locale_z(locale);
    return ffi_string_call([&](char *buf, int len) {
        return icu4x_titlecase_segment(src.data(), static_cast<int>(src.size()), locale_z.c_str(), buf, len);
    });
}

// ── Normalization ────────────────────────────────────────────────────

std::string normalize(std::string_view src, normalization_form form) {
    return ffi_string_call([&](char *buf, int len) {
        return icu4x_normalize(src.data(), static_cast<int>(src.size()), static_cast<int>(form), buf, len);
    });
}

bool is_normalized(std::string_view src, normalization_form form) {
    int result = icu4x_is_normalized(src.data(), static_cast<int>(src.size()), static_cast<int>(form));
    return result == 1;
}

// ── Character Properties ─────────────────────────────────────────────

bool code_point_has_property(char32_t cp, unicode_property prop) {
    int result = icu4x_code_point_has_property(static_cast<unsigned int>(cp), static_cast<int>(prop));
    return result == 1;
}

// ── Segmentation ─────────────────────────────────────────────────────

std::vector<size_t> segment(std::string_view src, segmentation_type type) {
    auto *handle = icu4x_segment(src.data(), static_cast<int>(src.size()), static_cast<int>(type));
    if (!handle) return {};

    int count = icu4x_break_indices_count(handle);
    std::vector<size_t> result;
    result.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        int offset = icu4x_break_indices_get(handle, i);
        if (offset >= 0) {
            result.push_back(static_cast<size_t>(offset));
        }
    }
    icu4x_break_indices_destroy(handle);
    return result;
}

// ── Collation ────────────────────────────────────────────────────────

int collation_compare(std::string_view a, std::string_view b, std::string_view locale, collation_strength strength) {
    std::string locale_z(locale);
    return icu4x_collation_compare(a.data(), static_cast<int>(a.size()), b.data(), static_cast<int>(b.size()),
                                   locale_z.c_str(), static_cast<int>(strength));
}

int collation_find(std::string_view haystack, std::string_view needle, std::string_view locale,
                   collation_strength strength) {
    std::string locale_z(locale);
    return icu4x_collation_contains(haystack.data(), static_cast<int>(haystack.size()), needle.data(),
                                    static_cast<int>(needle.size()), locale_z.c_str(), static_cast<int>(strength));
}

bool collation_contains(std::string_view haystack, std::string_view needle, std::string_view locale,
                        collation_strength strength) {
    return collation_find(haystack, needle, locale, strength) >= 0;
}

// ── Transliteration ──────────────────────────────────────────────────

std::string transliterate(std::string_view src, std::string_view transform_id) {
    std::string id_z(transform_id);
    return ffi_string_call([&](char *buf, int len) {
        return icu4x_transliterate(src.data(), static_cast<int>(src.size()), id_z.c_str(), buf, len);
    });
}

} // namespace icetea
