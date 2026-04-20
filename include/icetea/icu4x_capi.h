#pragma once

/// @file icu4x_capi.h
/// @brief C FFI declarations for the ICU4X Rust static library.
///
/// These functions are implemented in Rust (rust/icu4x_capi/src/lib.rs)
/// and linked as a static library (icu4x_capi.lib).

#ifdef __cplusplus
extern "C" {
#endif

/// Opaque handle to the ICU4X bridge state (Rust-owned).
typedef struct Icu4xBridge Icu4xBridge;

/// Create a new ICU4X bridge for the given BCP-47 locale tag.
/// Returns NULL on failure. Caller must free with icu4x_bridge_destroy().
Icu4xBridge *icu4x_bridge_create(const char *locale_str);

/// Destroy an ICU4X bridge instance.
void icu4x_bridge_destroy(Icu4xBridge *bridge);

/// Get the cardinal plural category for a number.
/// Returns 0=zero, 1=one, 2=two, 3=few, 4=many, 5=other, -1=error.
int icu4x_cardinal_category(const Icu4xBridge *bridge, double n);

/// Get the ordinal plural category for a number.
/// Returns 0=zero, 1=one, 2=two, 3=few, 4=many, 5=other, -1=error.
int icu4x_ordinal_category(const Icu4xBridge *bridge, double n);

/// Format a number using the locale's decimal formatter.
/// Writes into out_buf (max out_buf_len bytes including null).
/// Returns bytes written (excl null), or required size if too small, or -1 on error.
int icu4x_format_number(const Icu4xBridge *bridge, double n, int fraction_digits, char *out_buf, int out_buf_len);

/// Get the locale string of the bridge.
/// Returns bytes written (excl null), or required size if too small, or -1 on error.
int icu4x_get_locale(const Icu4xBridge *bridge, char *out_buf, int out_buf_len);

/// Format a date (year, month, day) using the locale's date formatter.
/// style: 0=short, 1=medium, 2=long.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_format_date(const Icu4xBridge *bridge, int year, int month, int day, int style, char *out_buf,
                      int out_buf_len);

/// Format a time (hour, minute, second) using the locale's time formatter.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_format_time(const Icu4xBridge *bridge, int hour, int minute, int second, char *out_buf, int out_buf_len);

/// Format a list of strings.
/// list_type: 0=and, 1=or, 2=unit.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_format_list(const Icu4xBridge *bridge, const char *const *items, int item_count, int list_type, char *out_buf,
                      int out_buf_len);

/// Format a currency amount with a 3-letter ISO 4217 code.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_format_currency(const Icu4xBridge *bridge, double amount, const char *currency_code, char *out_buf,
                          int out_buf_len);

/// Format a relative time value.
/// value: signed offset (negative=past, positive=future).
/// unit: 0=second, 1=minute, 2=hour, 3=day, 4=week, 5=month, 6=year.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_format_reltime(const Icu4xBridge *bridge, double value, int unit, char *out_buf, int out_buf_len);

/// Get the cardinal plural category for a range of numbers.
/// Returns 0=zero, 1=one, 2=two, 3=few, 4=many, 5=other, -1=error.
int icu4x_cardinal_range(const Icu4xBridge *bridge, double start, double end);

// ═══════════════════════════════════════════════════════════════════════
//  Phase 4: Case Mapping (locale-independent of Icu4xBridge)
// ═══════════════════════════════════════════════════════════════════════

/// Convert a UTF-8 string to lowercase using the given locale.
/// src/src_len: input UTF-8 string (not necessarily null-terminated).
/// locale_str: null-terminated BCP-47 locale tag (e.g. "en", "tr").
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_lowercase(const char *src, int src_len, const char *locale_str, char *out_buf, int out_buf_len);

/// Convert a UTF-8 string to uppercase using the given locale.
int icu4x_uppercase(const char *src, int src_len, const char *locale_str, char *out_buf, int out_buf_len);

/// Case-fold a UTF-8 string (locale-independent, for case-insensitive comparison).
int icu4x_case_fold(const char *src, int src_len, char *out_buf, int out_buf_len);

/// Titlecase a UTF-8 string segment using the given locale.
/// Treats the entire input as a single segment.
int icu4x_titlecase_segment(const char *src, int src_len, const char *locale_str, char *out_buf, int out_buf_len);

// ═══════════════════════════════════════════════════════════════════════
//  Phase 4: Unicode Normalization
// ═══════════════════════════════════════════════════════════════════════

/// Normalization form constants.
#define ICU4X_NORM_NFC 0
#define ICU4X_NORM_NFD 1
#define ICU4X_NORM_NFKC 2
#define ICU4X_NORM_NFKD 3

/// Normalize a UTF-8 string to the given Unicode Normalization Form.
/// form: ICU4X_NORM_NFC/NFD/NFKC/NFKD.
/// Returns bytes written (excl null), required size if too small, or -1 on error.
int icu4x_normalize(const char *src, int src_len, int form, char *out_buf, int out_buf_len);

/// Check if a UTF-8 string is in the given normalization form.
/// Returns 1 if normalized, 0 if not, -1 on error.
int icu4x_is_normalized(const char *src, int src_len, int form);

// ═══════════════════════════════════════════════════════════════════════
//  Phase 4: Unicode Properties (code point queries)
// ═══════════════════════════════════════════════════════════════════════

/// Property constants for icu4x_code_point_has_property.
#define ICU4X_PROP_ALPHABETIC 0
#define ICU4X_PROP_WHITESPACE 1
#define ICU4X_PROP_NUMERIC 2
#define ICU4X_PROP_LOWERCASE 3
#define ICU4X_PROP_UPPERCASE 4
#define ICU4X_PROP_EMOJI 5
#define ICU4X_PROP_IDEOGRAPHIC 6

/// Check if a Unicode code point has a specific property.
/// Returns 1 if true, 0 if false, -1 on invalid property.
int icu4x_code_point_has_property(unsigned int code_point, int property);

// ═══════════════════════════════════════════════════════════════════════
//  Phase 4: Unicode Segmentation (grapheme, word, sentence)
// ═══════════════════════════════════════════════════════════════════════

/// Opaque handle for segmentation break indices.
typedef struct Icu4xBreakIndices Icu4xBreakIndices;

/// Segmenter type constants.
#define ICU4X_SEG_GRAPHEME 0
#define ICU4X_SEG_WORD 1
#define ICU4X_SEG_SENTENCE 2

/// Segment a UTF-8 string and return an opaque handle to the break indices.
/// seg_type: ICU4X_SEG_GRAPHEME/WORD/SENTENCE.
/// Returns NULL on error. Caller must free with icu4x_break_indices_destroy().
Icu4xBreakIndices *icu4x_segment(const char *src, int src_len, int seg_type);

/// Get the number of break positions.
int icu4x_break_indices_count(const Icu4xBreakIndices *handle);

/// Get a break position by index. Returns the byte offset, or -1 if out of bounds.
int icu4x_break_indices_get(const Icu4xBreakIndices *handle, int index);

/// Destroy a break indices handle.
void icu4x_break_indices_destroy(Icu4xBreakIndices *handle);

// ═══════════════════════════════════════════════════════════════════════
//  Phase 6.2: Collation (locale-aware string comparison and search)
// ═══════════════════════════════════════════════════════════════════════

/// Collation strength constants.
#define ICU4X_COLLATION_PRIMARY 0   // base letters (accent+case insensitive)
#define ICU4X_COLLATION_SECONDARY 1 // + accents (case insensitive)
#define ICU4X_COLLATION_TERTIARY 2  // + case (default)

/// Compare two UTF-8 strings using locale-aware collation.
/// strength: ICU4X_COLLATION_PRIMARY/SECONDARY/TERTIARY.
/// Returns -1 if a < b, 0 if a == b, 1 if a > b, -2 on error.
int icu4x_collation_compare(const char *a_src, int a_len, const char *b_src, int b_len, const char *locale_str,
                            int strength);

/// Search for needle in haystack using locale-aware collation.
/// Returns the byte offset of the first match, -1 if not found, -2 on error.
int icu4x_collation_contains(const char *haystack_src, int haystack_len, const char *needle_src, int needle_len,
                             const char *locale_str, int strength);

// ═══════════════════════════════════════════════════════════════════════
//  Transliteration (ICU4X experimental — API may change)
// ═══════════════════════════════════════════════════════════════════════

/// Transliterate a UTF-8 string using a BCP-47 transform locale ID.
/// transform_id: null-terminated BCP-47 transform ID, e.g.:
///   "und-Hans-t-und-hant" (Traditional → Simplified Chinese)
///   "und-Hant-t-und-hans" (Simplified → Traditional Chinese)
/// Returns bytes written (excl null), required size if too small, or -1 on error.
///
/// NOTE: Uses ICU4X experimental Transliterator — unstable, may change in future releases.
int icu4x_transliterate(const char *src, int src_len, const char *transform_id, char *out_buf, int out_buf_len);

#ifdef __cplusplus
}
#endif
