/* icu4x_stubs.c -- No-op stubs for ICU4X C API when Rust library is not linked */

#include <stddef.h>

typedef struct Icu4xBridge Icu4xBridge;

Icu4xBridge* icu4x_bridge_create(const char* locale_str) {
    (void)locale_str;
    return NULL;
}

void icu4x_bridge_destroy(Icu4xBridge* bridge) {
    (void)bridge;
}

int icu4x_cardinal_category(const Icu4xBridge* bridge, double n) {
    (void)bridge; (void)n;
    return 5; /* other */
}

int icu4x_ordinal_category(const Icu4xBridge* bridge, double n) {
    (void)bridge; (void)n;
    return 5; /* other */
}

int icu4x_format_number(const Icu4xBridge* bridge, double n, int fraction_digits,
                        char* out_buf, int out_buf_len) {
    (void)bridge; (void)n; (void)fraction_digits; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_get_locale(const Icu4xBridge* bridge, char* out_buf, int out_buf_len) {
    (void)bridge; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_format_date(const Icu4xBridge* bridge, int year, int month, int day,
                      int style, char* out_buf, int out_buf_len) {
    (void)bridge; (void)year; (void)month; (void)day; (void)style; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_format_time(const Icu4xBridge* bridge, int hour, int minute, int second,
                      char* out_buf, int out_buf_len) {
    (void)bridge; (void)hour; (void)minute; (void)second; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_format_list(const Icu4xBridge* bridge, const char* const* items,
                      int item_count, int list_type, char* out_buf, int out_buf_len) {
    (void)bridge; (void)items; (void)item_count; (void)list_type; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_format_currency(const Icu4xBridge* bridge, double amount,
                          const char* currency_code, char* out_buf, int out_buf_len) {
    (void)bridge; (void)amount; (void)currency_code; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_format_reltime(const Icu4xBridge* bridge, double value, int unit,
                         char* out_buf, int out_buf_len) {
    (void)bridge; (void)value; (void)unit; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_cardinal_range(const Icu4xBridge* bridge, double start, double end) {
    (void)bridge; (void)start; (void)end;
    return 5; /* other */
}

/* Phase 4: Case Mapping stubs */

int icu4x_lowercase(const char* src, int src_len, const char* locale_str,
                    char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)locale_str; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_uppercase(const char* src, int src_len, const char* locale_str,
                    char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)locale_str; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_case_fold(const char* src, int src_len,
                    char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_titlecase_segment(const char* src, int src_len, const char* locale_str,
                            char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)locale_str; (void)out_buf; (void)out_buf_len;
    return -1;
}

/* Phase 4: Normalization stubs */

int icu4x_normalize(const char* src, int src_len, int form,
                    char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)form; (void)out_buf; (void)out_buf_len;
    return -1;
}

int icu4x_is_normalized(const char* src, int src_len, int form) {
    (void)src; (void)src_len; (void)form;
    return -1;
}

/* Phase 4: Properties stubs */

typedef struct Icu4xBreakIndices Icu4xBreakIndices;

int icu4x_code_point_has_property(unsigned int code_point, int property) {
    (void)code_point; (void)property;
    return -1;
}

/* Phase 4: Segmentation stubs */

Icu4xBreakIndices* icu4x_segment(const char* src, int src_len, int seg_type) {
    (void)src; (void)src_len; (void)seg_type;
    return NULL;
}

int icu4x_break_indices_count(const Icu4xBreakIndices* handle) {
    (void)handle;
    return 0;
}

int icu4x_break_indices_get(const Icu4xBreakIndices* handle, int index) {
    (void)handle; (void)index;
    return -1;
}

void icu4x_break_indices_destroy(Icu4xBreakIndices* handle) {
    (void)handle;
}

/* Phase 4: Collation stubs */

int icu4x_collation_compare(const char* a_src, int a_len,
                            const char* b_src, int b_len,
                            const char* locale_str, int strength) {
    (void)a_src; (void)a_len; (void)b_src; (void)b_len;
    (void)locale_str; (void)strength;
    return -2; /* error: not available */
}

int icu4x_collation_contains(const char* haystack_src, int haystack_len,
                             const char* needle_src, int needle_len,
                             const char* locale_str, int strength) {
    (void)haystack_src; (void)haystack_len; (void)needle_src; (void)needle_len;
    (void)locale_str; (void)strength;
    return -2; /* error: not available */
}

/* Phase 4: Transliteration stub */

int icu4x_transliterate(const char* src, int src_len, const char* transform_id,
                        char* out_buf, int out_buf_len) {
    (void)src; (void)src_len; (void)transform_id; (void)out_buf; (void)out_buf_len;
    return -1; /* error: not available */
}
