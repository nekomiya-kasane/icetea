#pragma once

/// @file unicode_utils.h
/// @brief Stateless Unicode utility functions wrapping ICU4X C FFI.
///
/// These functions do NOT require an Icu4xBridge handle — they use ICU4X's
/// compiled data directly. Covers case mapping, normalization, character
/// properties, and text segmentation.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "icetea/exports.h"
#include "icetea/icu4x_capi.h"

namespace icetea {

// ── Case Mapping ─────────────────────────────────────────────────────

/// Convert a UTF-8 string to lowercase using the given BCP-47 locale.
[[nodiscard]] ICETEA_API std::string lowercase(std::string_view src, std::string_view locale);

/// Convert a UTF-8 string to uppercase using the given BCP-47 locale.
[[nodiscard]] ICETEA_API std::string uppercase(std::string_view src, std::string_view locale);

/// Case-fold a UTF-8 string (locale-independent, for case-insensitive comparison).
[[nodiscard]] ICETEA_API std::string case_fold(std::string_view src);

/// Titlecase a UTF-8 string segment using the given BCP-47 locale.
/// Treats the entire input as a single segment.
[[nodiscard]] ICETEA_API std::string titlecase_segment(std::string_view src, std::string_view locale);

// ── Normalization ────────────────────────────────────────────────────

enum class normalization_form : int {
    nfc  = ICU4X_NORM_NFC,
    nfd  = ICU4X_NORM_NFD,
    nfkc = ICU4X_NORM_NFKC,
    nfkd = ICU4X_NORM_NFKD,
};

/// Normalize a UTF-8 string to the given Unicode Normalization Form.
[[nodiscard]] ICETEA_API std::string normalize(std::string_view src, normalization_form form);

/// Check if a UTF-8 string is already in the given normalization form.
[[nodiscard]] ICETEA_API bool is_normalized(std::string_view src, normalization_form form);

// ── Character Properties ─────────────────────────────────────────────

enum class unicode_property : int {
    alphabetic  = ICU4X_PROP_ALPHABETIC,
    whitespace  = ICU4X_PROP_WHITESPACE,
    numeric     = ICU4X_PROP_NUMERIC,
    lowercase   = ICU4X_PROP_LOWERCASE,
    uppercase   = ICU4X_PROP_UPPERCASE,
    emoji       = ICU4X_PROP_EMOJI,
    ideographic = ICU4X_PROP_IDEOGRAPHIC,
};

/// Check if a Unicode code point has a specific property.
[[nodiscard]] ICETEA_API bool code_point_has_property(char32_t cp, unicode_property prop);

// ── Segmentation ─────────────────────────────────────────────────────

enum class segmentation_type : int {
    grapheme = ICU4X_SEG_GRAPHEME,
    word     = ICU4X_SEG_WORD,
    sentence = ICU4X_SEG_SENTENCE,
};

/// Segment a UTF-8 string and return the list of byte offsets at break positions.
[[nodiscard]] ICETEA_API std::vector<size_t> segment(std::string_view src, segmentation_type type);

// ── Collation ────────────────────────────────────────────────────────

enum class collation_strength : int {
    primary   = ICU4X_COLLATION_PRIMARY,   // base letters (accent+case insensitive)
    secondary = ICU4X_COLLATION_SECONDARY, // + accents (case insensitive)
    tertiary  = ICU4X_COLLATION_TERTIARY,  // + case (default)
};

/// Compare two UTF-8 strings using locale-aware collation.
/// Returns -1 (a < b), 0 (a == b), or 1 (a > b).
[[nodiscard]] ICETEA_API int collation_compare(std::string_view a, std::string_view b,
                                               std::string_view locale,
                                               collation_strength strength = collation_strength::tertiary);

/// Search for needle in haystack using locale-aware collation.
/// Returns the byte offset of the first match, or -1 if not found.
[[nodiscard]] ICETEA_API int collation_find(std::string_view haystack, std::string_view needle,
                                            std::string_view locale,
                                            collation_strength strength = collation_strength::tertiary);

/// Check if haystack contains needle using locale-aware collation.
[[nodiscard]] ICETEA_API bool collation_contains(std::string_view haystack, std::string_view needle,
                                                 std::string_view locale,
                                                 collation_strength strength = collation_strength::tertiary);

// ── Transliteration (ICU4X experimental — API may change) ───────────

/// Transliterate a UTF-8 string using a BCP-47 transform locale ID.
/// @param src       Input UTF-8 string.
/// @param transform_id  BCP-47 transform ID, e.g.:
///   "und-Hans-t-und-hant" (Traditional → Simplified Chinese)
///   "und-Hant-t-und-hans" (Simplified → Traditional Chinese)
/// @return Transliterated UTF-8 string, or empty string on error.
///
/// @note Uses ICU4X experimental Transliterator — unstable, may change in future releases.
[[nodiscard]] ICETEA_API std::string transliterate(std::string_view src,
                                                    std::string_view transform_id);

}  // namespace icetea
