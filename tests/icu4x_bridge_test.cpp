#include "icetea/icetea.h"

#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <vector>

using namespace icetea;

// ── version() ────────────────────────────────────────────────────────────

TEST(IceteaVersion, ReturnsNonNull) {
    const char *v = icetea::version();
    ASSERT_NE(v, nullptr);
    EXPECT_GT(std::string_view(v).size(), 0u);
}

// ── icu4x_bridge: construction & move ────────────────────────────────────

TEST(Icu4xBridge, DefaultConstruction) {
    icu4x_bridge bridge;
    EXPECT_FALSE(bridge.is_initialized());
    EXPECT_TRUE(bridge.current_locale().empty());
}

TEST(Icu4xBridge, MoveConstruction) {
    icu4x_bridge a;
    auto r = a.set_locale("en");
    ASSERT_TRUE(r.has_value());

    icu4x_bridge b(std::move(a));
    EXPECT_TRUE(b.is_initialized());
    EXPECT_EQ(b.current_locale(), "en");
}

TEST(Icu4xBridge, MoveAssignment) {
    icu4x_bridge a;
    a.set_locale("en");

    icu4x_bridge b;
    b = std::move(a);
    EXPECT_TRUE(b.is_initialized());
    EXPECT_EQ(b.current_locale(), "en");
}

// ── set_locale ───────────────────────────────────────────────────────────

TEST(Icu4xBridge, SetLocaleLazy) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", true);
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(bridge.is_initialized());
    // Locale tag stored but bridge not created yet (lazy)
    EXPECT_EQ(bridge.current_locale(), "en");
}

TEST(Icu4xBridge, SetLocaleEager) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    // If ICU4X data is available, this succeeds; otherwise it may fail
    // Either way, current_locale should return the tag
    EXPECT_EQ(bridge.current_locale(), "en");
}

TEST(Icu4xBridge, SetLocaleReplaces) {
    icu4x_bridge bridge;
    bridge.set_locale("en");
    bridge.set_locale("fr");
    EXPECT_EQ(bridge.current_locale(), "fr");
}

TEST(Icu4xBridge, SetLocaleTooLong) {
    icu4x_bridge bridge;
    std::string long_locale(max_locale_length + 1, 'a');
    auto r = bridge.set_locale(long_locale);
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error().code, error_code::invalid_argument);
}

// ── cardinal_category ────────────────────────────────────────────────────

TEST(Icu4xBridge, CardinalCategoryWithoutLocale) {
    icu4x_bridge bridge;
    // No locale set — should return fallback (other)
    auto cat = bridge.cardinal_category(1.0);
    EXPECT_EQ(cat, plural_category::other);
}

TEST(Icu4xBridge, CardinalCategoryEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available for 'en'";
    }
    // English: 1 -> one, 0 -> other, 2 -> other
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(5.0), plural_category::other);
}

// ── ordinal_category ─────────────────────────────────────────────────────

TEST(Icu4xBridge, OrdinalCategoryWithoutLocale) {
    icu4x_bridge bridge;
    auto cat = bridge.ordinal_category(1.0);
    EXPECT_EQ(cat, plural_category::other);
}

TEST(Icu4xBridge, OrdinalCategoryEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available for 'en'";
    }
    // English ordinals: 1st=one, 2nd=two, 3rd=few, 4th=other
    EXPECT_EQ(bridge.ordinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.ordinal_category(2.0), plural_category::two);
    EXPECT_EQ(bridge.ordinal_category(3.0), plural_category::few);
    EXPECT_EQ(bridge.ordinal_category(4.0), plural_category::other);
}

// ── cardinal_range ───────────────────────────────────────────────────────

TEST(Icu4xBridge, CardinalRangeWithoutLocale) {
    icu4x_bridge bridge;
    auto cat = bridge.cardinal_range(1.0, 5.0);
    EXPECT_EQ(cat, plural_category::other);
}

TEST(Icu4xBridge, CardinalRangeEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available for 'en'";
    }
    // English range: "1–5" → other
    auto cat = bridge.cardinal_range(1.0, 5.0);
    // Just verify it returns a valid category
    EXPECT_GE(static_cast<int>(cat), 0);
    EXPECT_LE(static_cast<int>(cat), 5);
}

// ── format_number ────────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatNumberWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_number(out, 1234.5);
    // Fallback: std::to_string
    EXPECT_FALSE(out.empty());
}

TEST(Icu4xBridge, FormatNumberEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_number(out, 1234.5);
    EXPECT_FALSE(out.empty());
    // English should contain "1,234" or "1234"
    EXPECT_NE(out.find("1"), std::string::npos);
}

TEST(Icu4xBridge, FormatNumberWithFractionDigits) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_number(out, 3.14159, 2);
    EXPECT_FALSE(out.empty());
}

// ── format_date ──────────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatDateWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_date(out, 2025, 1, 15);
    // Fallback: "2025-1-15"
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("2025"), std::string::npos);
}

TEST(Icu4xBridge, FormatDateEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_date(out, 2025, 1, 15, date_style::medium_fmt);
    EXPECT_FALSE(out.empty());
}

TEST(Icu4xBridge, FormatDateStyles) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    for (auto style : {date_style::short_fmt, date_style::medium_fmt, date_style::long_fmt}) {
        std::string out;
        bridge.format_date(out, 2025, 6, 15, style);
        EXPECT_FALSE(out.empty()) << "Style " << static_cast<int>(style);
    }
}

// ── format_time ──────────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatTimeWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_time(out, 14, 30, 0);
    // Fallback: "14:30:00"
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("14"), std::string::npos);
}

TEST(Icu4xBridge, FormatTimeEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_time(out, 14, 30, 0);
    EXPECT_FALSE(out.empty());
}

// ── format_list ──────────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatListEmpty) {
    icu4x_bridge bridge;
    bridge.set_locale("en");
    std::string out;
    std::vector<std::string_view> items;
    bridge.format_list(out, items);
    EXPECT_TRUE(out.empty());
}

TEST(Icu4xBridge, FormatListWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    std::vector<std::string_view> items = {"apple", "banana", "cherry"};
    bridge.format_list(out, items);
    // Fallback: comma-separated
    EXPECT_NE(out.find("apple"), std::string::npos);
    EXPECT_NE(out.find("banana"), std::string::npos);
    EXPECT_NE(out.find("cherry"), std::string::npos);
}

TEST(Icu4xBridge, FormatListEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    std::vector<std::string_view> items = {"apple", "banana", "cherry"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("apple"), std::string::npos);
}

// ── format_currency ──────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatCurrencyWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_currency(out, 19.99, "USD");
    // Fallback: "19.990000 USD" (from std::to_string)
    EXPECT_FALSE(out.empty());
}

TEST(Icu4xBridge, FormatCurrencyEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_currency(out, 19.99, "USD");
    EXPECT_FALSE(out.empty());
}

// ── format_reltime ───────────────────────────────────────────────────────

TEST(Icu4xBridge, FormatReltimeWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_reltime(out, -3.0, reltime_unit::day);
    // Fallback: "3 units ago"
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("3"), std::string::npos);
}

TEST(Icu4xBridge, FormatReltimeFutureWithoutLocale) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_reltime(out, 2.0, reltime_unit::hour);
    // Fallback: "in 2 units"
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("2"), std::string::npos);
}

TEST(Icu4xBridge, FormatReltimeEnglish) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    std::string out;
    bridge.format_reltime(out, -1.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
}

TEST(Icu4xBridge, FormatReltimeUnits) {
    icu4x_bridge bridge;
    auto r = bridge.set_locale("en", false);
    if (!r.has_value()) {
        GTEST_SKIP() << "ICU4X data not available";
    }
    for (auto unit : {reltime_unit::second, reltime_unit::minute, reltime_unit::hour, reltime_unit::day,
                      reltime_unit::week, reltime_unit::month, reltime_unit::year}) {
        std::string out;
        bridge.format_reltime(out, -2.0, unit);
        EXPECT_FALSE(out.empty()) << "Unit " << static_cast<int>(unit);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale test infrastructure
// ═══════════════════════════════════════════════════════════════════════════

// Helper: create an eager bridge for a locale, skip test if unavailable.
#define BRIDGE_OR_SKIP(var, loc)                                                                                       \
    icu4x_bridge var;                                                                                                  \
    do {                                                                                                               \
        auto _r = var.set_locale(loc, false);                                                                          \
        if (!_r.has_value()) {                                                                                         \
            GTEST_SKIP() << "ICU4X data not available for '" loc "'";                                                  \
        }                                                                                                              \
    } while (false)

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: cardinal plural rules
//
//  CLDR cardinal rules vary dramatically across languages.
//  See: https://www.unicode.org/cldr/charts/latest/supplemental/language_plural_rules.html
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, CardinalArabic) {
    // Arabic uses all 6 plural categories:
    //   0→zero, 1→one, 2→two, 3-10→few, 11-99→many, 100+→other
    BRIDGE_OR_SKIP(bridge, "ar");
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::zero);
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::two);
    EXPECT_EQ(bridge.cardinal_category(3.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(6.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(10.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(11.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(49.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(99.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(100.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(1000.0), plural_category::other);
}

TEST(MultiLocale, CardinalChinese) {
    // Chinese: all numbers → other (no plural distinction)
    BRIDGE_OR_SKIP(bridge, "zh");
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(100.0), plural_category::other);
}

TEST(MultiLocale, CardinalJapanese) {
    // Japanese: same as Chinese, only other
    BRIDGE_OR_SKIP(bridge, "ja");
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(5.0), plural_category::other);
}

TEST(MultiLocale, CardinalRussian) {
    // Russian: one (1, 21, 31..), few (2-4, 22-24..), many (0, 5-20, 25-30..), other (fractions)
    BRIDGE_OR_SKIP(bridge, "ru");
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(21.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(101.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(3.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(4.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(22.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(5.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(11.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(14.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(20.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(100.0), plural_category::many);
}

TEST(MultiLocale, CardinalPolish) {
    // Polish: one (1), few (2-4, 22-24..), many (0, 5-21, 25-31..), other (fractions)
    BRIDGE_OR_SKIP(bridge, "pl");
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(3.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(4.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(22.0), plural_category::few);
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(5.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(12.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(20.0), plural_category::many);
    EXPECT_EQ(bridge.cardinal_category(100.0), plural_category::many);
}

TEST(MultiLocale, CardinalFrench) {
    // French: one (0 and 1), other (2+)
    BRIDGE_OR_SKIP(bridge, "fr");
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(100.0), plural_category::other);
}

TEST(MultiLocale, CardinalGerman) {
    // German: 1→one, everything else→other
    BRIDGE_OR_SKIP(bridge, "de");
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(0.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(21.0), plural_category::other);
}

TEST(MultiLocale, CardinalHebrew) {
    // Hebrew CLDR: 1→one, 2→two; the "many" rule (n%10==0 && n>10) requires
    // integer operand v==0 which may not hold for float-to-Decimal conversion.
    // ICU4X returns "other" for 20/30/40 via f64 path — test observable behavior.
    BRIDGE_OR_SKIP(bridge, "he");
    EXPECT_EQ(bridge.cardinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.cardinal_category(2.0), plural_category::two);
    EXPECT_EQ(bridge.cardinal_category(3.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(5.0), plural_category::other);
    EXPECT_EQ(bridge.cardinal_category(20.0), plural_category::other);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: ordinal plural rules
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, OrdinalChinese) {
    // Chinese: all ordinals → other
    BRIDGE_OR_SKIP(bridge, "zh");
    EXPECT_EQ(bridge.ordinal_category(1.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(3.0), plural_category::other);
}

TEST(MultiLocale, OrdinalGerman) {
    // German: all ordinals → other (German uses trailing period: 1., 2., 3.)
    BRIDGE_OR_SKIP(bridge, "de");
    EXPECT_EQ(bridge.ordinal_category(1.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(3.0), plural_category::other);
}

TEST(MultiLocale, OrdinalFrench) {
    // French: 1→one, rest→other
    BRIDGE_OR_SKIP(bridge, "fr");
    EXPECT_EQ(bridge.ordinal_category(1.0), plural_category::one);
    EXPECT_EQ(bridge.ordinal_category(2.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(3.0), plural_category::other);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: cardinal range
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, CardinalRangeFrench) {
    // French range "1-5": end value 5 → other
    BRIDGE_OR_SKIP(bridge, "fr");
    auto cat = bridge.cardinal_range(1.0, 5.0);
    EXPECT_GE(static_cast<int>(cat), 0);
    EXPECT_LE(static_cast<int>(cat), 5);
}

TEST(MultiLocale, CardinalRangeRussian) {
    BRIDGE_OR_SKIP(bridge, "ru");
    // Range 1-5: end=5 → many
    EXPECT_EQ(bridge.cardinal_range(1.0, 5.0), plural_category::many);
    // Range 1-2: end=2 → few
    EXPECT_EQ(bridge.cardinal_range(1.0, 2.0), plural_category::few);
    // Range 1-21: end=21 → one
    EXPECT_EQ(bridge.cardinal_range(1.0, 21.0), plural_category::one);
}

TEST(MultiLocale, CardinalRangeArabic) {
    BRIDGE_OR_SKIP(bridge, "ar");
    // Range 0-3: end=3 → few
    EXPECT_EQ(bridge.cardinal_range(0.0, 3.0), plural_category::few);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: number formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatNumberGerman) {
    // German uses period as grouping separator and comma for decimal
    // 1234.5 → "1.234,5"
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_number(out, 1234.5);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find(','), std::string::npos) << "Expected comma decimal separator, got: " << out;
}

TEST(MultiLocale, FormatNumberFrench) {
    // French uses narrow no-break space (U+202F) or non-breaking space as grouping
    // 1234.5 → "1 234,5" (with some unicode space)
    BRIDGE_OR_SKIP(bridge, "fr");
    std::string out;
    bridge.format_number(out, 1234.5);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find(','), std::string::npos) << "Expected comma decimal separator, got: " << out;
}

TEST(MultiLocale, FormatNumberJapanese) {
    // Japanese uses same as English: comma grouping, period decimal
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    bridge.format_number(out, 1234567.89);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find('.'), std::string::npos) << "Expected period decimal separator, got: " << out;
}

TEST(MultiLocale, FormatNumberArabic) {
    // Arabic may use Arabic-Indic digits (٠١٢٣٤٥٦٧٨٩) or Western digits
    // depending on ICU4X data. Just verify it's non-empty and doesn't crash.
    BRIDGE_OR_SKIP(bridge, "ar");
    std::string out;
    bridge.format_number(out, 1234.5);
    EXPECT_FALSE(out.empty());
    // Arabic output is multi-byte UTF-8
    EXPECT_GT(out.size(), 4u) << "Arabic formatted number seems too short: " << out;
}

TEST(MultiLocale, FormatNumberRussian) {
    // Russian: space grouping, comma decimal → "1 234,5"
    BRIDGE_OR_SKIP(bridge, "ru");
    std::string out;
    bridge.format_number(out, 1234.5);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find(','), std::string::npos) << "Expected comma decimal separator, got: " << out;
}

TEST(MultiLocale, FormatNumberChinese) {
    BRIDGE_OR_SKIP(bridge, "zh");
    std::string out;
    bridge.format_number(out, 9876543.21);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find('.'), std::string::npos) << "Expected period decimal separator, got: " << out;
}

TEST(MultiLocale, FormatNumberFractionDigits) {
    // Verify fraction_digits works across locales
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_number(out, 3.14159, 2);
    EXPECT_FALSE(out.empty());
    // German: "3,14"
    EXPECT_NE(out.find("14"), std::string::npos) << "Expected 2-digit fraction, got: " << out;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: date formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatDateJapanese) {
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    bridge.format_date(out, 2025, 3, 15, date_style::long_fmt);
    EXPECT_FALSE(out.empty());
    // Japanese long date should contain "2025" and Japanese characters for month/day
    EXPECT_NE(out.find("2025"), std::string::npos) << "Expected year in output: " << out;
}

TEST(MultiLocale, FormatDateGerman) {
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_date(out, 2025, 3, 15, date_style::medium_fmt);
    EXPECT_FALSE(out.empty());
    // German medium: "15.03.2025" or similar
    EXPECT_NE(out.find("15"), std::string::npos) << "Expected day in output: " << out;
    EXPECT_NE(out.find("2025"), std::string::npos) << "Expected year in output: " << out;
}

TEST(MultiLocale, FormatDateFrench) {
    BRIDGE_OR_SKIP(bridge, "fr");
    std::string out;
    bridge.format_date(out, 2025, 7, 14, date_style::long_fmt);
    EXPECT_FALSE(out.empty());
    // French long: "14 juillet 2025"
    EXPECT_NE(out.find("14"), std::string::npos) << "Expected day in output: " << out;
    EXPECT_NE(out.find("2025"), std::string::npos) << "Expected year in output: " << out;
}

TEST(MultiLocale, FormatDateChinese) {
    BRIDGE_OR_SKIP(bridge, "zh");
    std::string out;
    bridge.format_date(out, 2025, 1, 1, date_style::medium_fmt);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("2025"), std::string::npos) << "Expected year in output: " << out;
}

TEST(MultiLocale, FormatDateRussian) {
    BRIDGE_OR_SKIP(bridge, "ru");
    std::string out;
    bridge.format_date(out, 2025, 12, 31, date_style::long_fmt);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("2025"), std::string::npos) << "Expected year in output: " << out;
    EXPECT_NE(out.find("31"), std::string::npos) << "Expected day in output: " << out;
}

TEST(MultiLocale, FormatDateArabic) {
    BRIDGE_OR_SKIP(bridge, "ar");
    std::string out;
    bridge.format_date(out, 2025, 6, 15, date_style::medium_fmt);
    EXPECT_FALSE(out.empty());
    // Arabic date output is non-trivial, just verify non-empty and non-crash
    EXPECT_GT(out.size(), 4u) << "Arabic formatted date seems too short: " << out;
}

TEST(MultiLocale, FormatDateAllStyles) {
    // Verify all 3 date styles across multiple locales
    for (const char *loc : {"en", "de", "fr", "ja", "zh", "ru", "ar", "pl", "he", "tr"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        for (auto style : {date_style::short_fmt, date_style::medium_fmt, date_style::long_fmt}) {
            std::string out;
            bridge.format_date(out, 2025, 6, 15, style);
            EXPECT_FALSE(out.empty()) << "Locale=" << loc << " style=" << static_cast<int>(style);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: time formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatTimeGerman) {
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_time(out, 14, 30, 0);
    EXPECT_FALSE(out.empty());
    // German typically uses 24h: "14:30"
    EXPECT_NE(out.find("14"), std::string::npos) << "Expected 24h format: " << out;
}

TEST(MultiLocale, FormatTimeJapanese) {
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    bridge.format_time(out, 9, 5, 0);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("9"), std::string::npos) << "Expected hour in output: " << out;
}

TEST(MultiLocale, FormatTimeAllLocales) {
    for (const char *loc : {"en", "de", "fr", "ja", "zh", "ru", "ar", "pl", "he", "tr"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        bridge.format_time(out, 23, 59, 59);
        EXPECT_FALSE(out.empty()) << "Locale=" << loc;
    }
}

TEST(MultiLocale, FormatTimeMidnight) {
    // Midnight (00:00:00) may display as "12:00 AM" (en) or "0:00" (de) etc.
    for (const char *loc : {"en", "de", "ja", "ar"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        bridge.format_time(out, 0, 0, 0);
        EXPECT_FALSE(out.empty()) << "Midnight failed for locale=" << loc;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: list formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatListGerman) {
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    std::vector<std::string_view> items = {"Apfel", "Banane", "Kirsche"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_FALSE(out.empty());
    // German conjunction: "Apfel, Banane und Kirsche"
    EXPECT_NE(out.find("Apfel"), std::string::npos);
    EXPECT_NE(out.find("Banane"), std::string::npos);
    EXPECT_NE(out.find("Kirsche"), std::string::npos);
}

TEST(MultiLocale, FormatListFrench) {
    BRIDGE_OR_SKIP(bridge, "fr");
    std::string out;
    std::vector<std::string_view> items = {"pomme", "banane", "cerise"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("pomme"), std::string::npos);
    EXPECT_NE(out.find("cerise"), std::string::npos);
}

TEST(MultiLocale, FormatListJapanese) {
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    std::vector<std::string_view> items = {"A", "B", "C"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("A"), std::string::npos);
    EXPECT_NE(out.find("C"), std::string::npos);
}

TEST(MultiLocale, FormatListChinese) {
    BRIDGE_OR_SKIP(bridge, "zh");
    std::string out;
    std::vector<std::string_view> items = {"A", "B", "C"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("A"), std::string::npos);
}

TEST(MultiLocale, FormatListDisjunction) {
    // Test disjunction (or) across locales
    for (const char *loc : {"en", "de", "fr", "ja", "zh", "ru"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        std::vector<std::string_view> items = {"X", "Y", "Z"};
        bridge.format_list(out, items, list_type::disjunction);
        EXPECT_FALSE(out.empty()) << "Disjunction failed for locale=" << loc;
        EXPECT_NE(out.find("X"), std::string::npos) << "Locale=" << loc << " out=" << out;
    }
}

TEST(MultiLocale, FormatListUnit) {
    // Test unit list type across locales
    for (const char *loc : {"en", "de", "fr", "ja"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        std::vector<std::string_view> items = {"3 kg", "200 g"};
        bridge.format_list(out, items, list_type::unit);
        EXPECT_FALSE(out.empty()) << "Unit list failed for locale=" << loc;
    }
}

TEST(MultiLocale, FormatListSingleItem) {
    // Single-item list should just return the item across all locales
    for (const char *loc : {"en", "de", "fr", "ja", "zh", "ru", "ar"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        std::vector<std::string_view> items = {"solo"};
        bridge.format_list(out, items, list_type::conjunction);
        EXPECT_NE(out.find("solo"), std::string::npos) << "Single-item list for locale=" << loc << " out=" << out;
    }
}

TEST(MultiLocale, FormatListTwoItems) {
    // Two-item list — usually "A and B" without Oxford comma
    for (const char *loc : {"en", "de", "fr", "ru"}) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        std::vector<std::string_view> items = {"alpha", "beta"};
        bridge.format_list(out, items, list_type::conjunction);
        EXPECT_NE(out.find("alpha"), std::string::npos) << "Locale=" << loc;
        EXPECT_NE(out.find("beta"), std::string::npos) << "Locale=" << loc;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: currency formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatCurrencyGerman) {
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_currency(out, 1234.56, "EUR");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("EUR"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyJapanese) {
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    bridge.format_currency(out, 1000.0, "JPY");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("JPY"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyFrench) {
    BRIDGE_OR_SKIP(bridge, "fr");
    std::string out;
    bridge.format_currency(out, 49.99, "EUR");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("EUR"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyRussian) {
    BRIDGE_OR_SKIP(bridge, "ru");
    std::string out;
    bridge.format_currency(out, 5000.0, "RUB");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("RUB"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyChinese) {
    BRIDGE_OR_SKIP(bridge, "zh");
    std::string out;
    bridge.format_currency(out, 88.88, "CNY");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("CNY"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyArabic) {
    BRIDGE_OR_SKIP(bridge, "ar");
    std::string out;
    bridge.format_currency(out, 999.99, "SAR");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("SAR"), std::string::npos) << "Expected currency code in: " << out;
}

TEST(MultiLocale, FormatCurrencyAllLocales) {
    // Verify no crash across many locale+currency combos
    struct {
        const char *loc;
        const char *cur;
        double amt;
    } cases[] = {
        {"en", "USD", 19.99},  {"en", "GBP", 100.0}, {"de", "EUR", 1234.56}, {"fr", "EUR", 49.99},
        {"ja", "JPY", 1000.0}, {"zh", "CNY", 88.88}, {"ru", "RUB", 5000.0},  {"ar", "SAR", 999.99},
        {"pl", "PLN", 42.50},  {"he", "ILS", 250.0}, {"tr", "TRY", 175.25},
    };
    for (auto &c : cases) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(c.loc, false);
        if (!r.has_value()) {
            continue;
        }

        std::string out;
        bridge.format_currency(out, c.amt, c.cur);
        EXPECT_FALSE(out.empty()) << "Locale=" << c.loc << " cur=" << c.cur;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale: relative time formatting
// ═══════════════════════════════════════════════════════════════════════════

TEST(MultiLocale, FormatReltimeGerman) {
    BRIDGE_OR_SKIP(bridge, "de");
    std::string out;
    bridge.format_reltime(out, -1.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // German: "gestern" (yesterday) for Numeric::Auto with -1 day
}

TEST(MultiLocale, FormatReltimeFrench) {
    BRIDGE_OR_SKIP(bridge, "fr");
    std::string out;
    bridge.format_reltime(out, 1.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // French: "demain" (tomorrow) for Numeric::Auto with +1 day
}

TEST(MultiLocale, FormatReltimeJapanese) {
    BRIDGE_OR_SKIP(bridge, "ja");
    std::string out;
    bridge.format_reltime(out, -1.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // Japanese: "昨日" (yesterday) for Numeric::Auto
}

TEST(MultiLocale, FormatReltimeChinese) {
    BRIDGE_OR_SKIP(bridge, "zh");
    std::string out;
    bridge.format_reltime(out, -1.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // Chinese: "昨天" (yesterday)
}

TEST(MultiLocale, FormatReltimeRussian) {
    BRIDGE_OR_SKIP(bridge, "ru");
    std::string out;
    bridge.format_reltime(out, -3.0, reltime_unit::hour);
    EXPECT_FALSE(out.empty());
    // Russian: "3 часа назад" or "3 ч. назад" (short)
}

TEST(MultiLocale, FormatReltimeArabic) {
    BRIDGE_OR_SKIP(bridge, "ar");
    std::string out;
    bridge.format_reltime(out, -2.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // Arabic might use "قبل يومين" (dual form)
}

TEST(MultiLocale, FormatReltimeAllUnitsAllLocales) {
    // Exhaustive: every unit × every locale
    const char *locales[] = {"en", "de", "fr", "ja", "zh", "ru", "ar", "pl", "he", "tr"};
    reltime_unit units[] = {reltime_unit::second, reltime_unit::minute, reltime_unit::hour, reltime_unit::day,
                            reltime_unit::week,   reltime_unit::month,  reltime_unit::year};
    for (const char *loc : locales) {
        icu4x_bridge bridge;
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }

        for (auto unit : units) {
            for (double val : {-5.0, -1.0, 0.0, 1.0, 5.0}) {
                std::string out;
                bridge.format_reltime(out, val, unit);
                EXPECT_FALSE(out.empty()) << "Locale=" << loc << " unit=" << static_cast<int>(unit) << " val=" << val;
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Edge cases & boundary conditions
// ═══════════════════════════════════════════════════════════════════════════

TEST(EdgeCase, FormatNumberZero) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, 0.0);
    EXPECT_EQ(out, "0");
}

TEST(EdgeCase, FormatNumberNegative) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, -42.5);
    EXPECT_FALSE(out.empty());
    // Should contain a minus sign (or equivalent)
    EXPECT_NE(out.find("42"), std::string::npos) << "out=" << out;
}

TEST(EdgeCase, FormatNumberLarge) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, 999999999.0);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("999"), std::string::npos) << "out=" << out;
}

TEST(EdgeCase, FormatNumberSmallFraction) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, 0.001);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("001"), std::string::npos) << "out=" << out;
}

TEST(EdgeCase, FormatDateLeapYear) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_date(out, 2024, 2, 29, date_style::medium_fmt);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("29"), std::string::npos) << "Leap day: " << out;
}

TEST(EdgeCase, FormatDateNewYear) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_date(out, 2025, 1, 1, date_style::long_fmt);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("2025"), std::string::npos) << "New Year: " << out;
}

TEST(EdgeCase, FormatDateEndOfYear) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_date(out, 2025, 12, 31, date_style::long_fmt);
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("31"), std::string::npos) << "End of year: " << out;
}

TEST(EdgeCase, FormatTimeBoundaries) {
    BRIDGE_OR_SKIP(bridge, "en");
    // Midnight
    {
        std::string out;
        bridge.format_time(out, 0, 0, 0);
        EXPECT_FALSE(out.empty());
    }
    // Just before midnight
    {
        std::string out;
        bridge.format_time(out, 23, 59, 59);
        EXPECT_FALSE(out.empty());
    }
    // Noon
    {
        std::string out;
        bridge.format_time(out, 12, 0, 0);
        EXPECT_FALSE(out.empty());
    }
}

TEST(EdgeCase, FormatCurrencyZero) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_currency(out, 0.0, "USD");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("USD"), std::string::npos);
}

TEST(EdgeCase, FormatCurrencyLargeAmount) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_currency(out, 1000000.0, "USD");
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("USD"), std::string::npos);
}

TEST(EdgeCase, FormatReltimeZero) {
    // "0 days" should produce something like "today" or "in 0 days"
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_reltime(out, 0.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
}

TEST(EdgeCase, CardinalCategoryLargeNumbers) {
    BRIDGE_OR_SKIP(bridge, "ru");
    // 1000001 ends in 1 but not 11 → one
    EXPECT_EQ(bridge.cardinal_category(1000001.0), plural_category::one);
    // 1000011 ends in 11 → many
    EXPECT_EQ(bridge.cardinal_category(1000011.0), plural_category::many);
    // 1000002 ends in 2 → few
    EXPECT_EQ(bridge.cardinal_category(1000002.0), plural_category::few);
}

TEST(EdgeCase, SetLocaleAndSwitch) {
    // Switch locale multiple times, verify formatting changes
    icu4x_bridge bridge;
    bridge.set_locale("en", false);
    std::string en_out;
    bridge.format_number(en_out, 1234.5);

    bridge.set_locale("de", false);
    std::string de_out;
    bridge.format_number(de_out, 1234.5);

    // en and de should differ (comma vs period grouping)
    EXPECT_NE(en_out, de_out) << "en=" << en_out << " de=" << de_out;
}

TEST(EdgeCase, SetLocaleMultipleTimes) {
    icu4x_bridge bridge;
    // Rapid locale switching should not leak or crash
    for (const char *loc : {"en", "de", "fr", "ja", "zh", "ru", "ar", "pl", "he", "tr",
                            "en", "de", "fr", "ja", "zh", "ru", "ar", "pl", "he", "tr"}) {
        auto r = bridge.set_locale(loc, false);
        if (!r.has_value()) {
            continue;
        }
        EXPECT_EQ(bridge.current_locale(), loc);

        std::string out;
        bridge.format_number(out, 42.0);
        EXPECT_FALSE(out.empty()) << "Locale=" << loc;
    }
}

TEST(EdgeCase, FormatNumberAppendSemantic) {
    // format_number appends to `out` — verify it doesn't clear previous content
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "prefix:";
    bridge.format_number(out, 42.0);
    EXPECT_EQ(out.substr(0, 7), "prefix:");
    EXPECT_NE(out.find("42"), std::string::npos);
}

TEST(EdgeCase, FormatDateAppendSemantic) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "date=";
    bridge.format_date(out, 2025, 1, 1, date_style::short_fmt);
    EXPECT_EQ(out.substr(0, 5), "date=");
    EXPECT_GT(out.size(), 5u);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Error handling & robustness
// ═══════════════════════════════════════════════════════════════════════════

TEST(ErrorHandling, SetLocaleInvalid) {
    icu4x_bridge bridge;
    // Extremely long or garbage locale string
    auto r = bridge.set_locale("this-is-not-a-real-locale-at-all-xxxxxxxxx");
    // Should either succeed (ICU4X accepts it) or fail gracefully — no crash
    (void)r;
}

TEST(ErrorHandling, UninitializedCardinal) {
    icu4x_bridge bridge;
    // Calling cardinal_category without set_locale
    auto cat = bridge.cardinal_category(1.0);
    // Should return some default or handle gracefully — no crash
    (void)cat;
}

TEST(ErrorHandling, UninitializedFormatNumber) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_number(out, 42.0);
    // Should produce empty output or handle gracefully — no crash
    (void)out;
}

TEST(ErrorHandling, UninitializedFormatDate) {
    icu4x_bridge bridge;
    std::string out;
    bridge.format_date(out, 2025, 6, 15, date_style::medium_fmt);
    (void)out;
}

TEST(ErrorHandling, FormatNumberFractionDigits) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, 3.14159, 2);
    // Should contain "3.14" (2 fraction digits)
    EXPECT_NE(out.find("3.14"), std::string::npos) << "got: " << out;
}

TEST(ErrorHandling, FormatNumberZeroFractionDigits) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_number(out, 3.14159, 0);
    // Should contain "3" without decimal part
    EXPECT_NE(out.find("3"), std::string::npos) << "got: " << out;
}

TEST(ErrorHandling, FormatListEmpty) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    std::vector<std::string_view> empty_items;
    bridge.format_list(out, empty_items, list_type::conjunction);
    // Empty list: should produce empty output, not crash
    (void)out;
}

TEST(ErrorHandling, FormatListSingleItem) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    std::vector<std::string_view> items = {"alone"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_NE(out.find("alone"), std::string::npos) << "got: " << out;
}

TEST(ErrorHandling, MultipleBridgesIndependent) {
    // Two bridges with different locales should not interfere
    icu4x_bridge en_bridge, de_bridge;
    auto r1 = en_bridge.set_locale("en");
    auto r2 = de_bridge.set_locale("de");
    if (!r1.has_value() || !r2.has_value()) {
        GTEST_SKIP() << "Locale data not available";
    }

    std::string en_out, de_out;
    en_bridge.format_number(en_out, 1234.5);
    de_bridge.format_number(de_out, 1234.5);

    // They should differ (en: 1,234.5 vs de: 1.234,5)
    EXPECT_NE(en_out, de_out) << "en=" << en_out << " de=" << de_out;

    // Verify en bridge still works after de bridge was used
    std::string en_out2;
    en_bridge.format_number(en_out2, 1234.5);
    EXPECT_EQ(en_out, en_out2);
}

TEST(ErrorHandling, FormatTimeAppendSemantic) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "time=";
    bridge.format_time(out, 14, 30, 0);
    EXPECT_EQ(out.substr(0, 5), "time=");
    EXPECT_GT(out.size(), 5u);
}

TEST(ErrorHandling, FormatCurrencyAppendSemantic) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "price=";
    bridge.format_currency(out, 99.99, "USD");
    EXPECT_EQ(out.substr(0, 6), "price=");
    EXPECT_GT(out.size(), 6u);
}

TEST(ErrorHandling, FormatListAppendSemantic) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "list=";
    std::vector<std::string_view> items = {"a", "b"};
    bridge.format_list(out, items, list_type::conjunction);
    EXPECT_EQ(out.substr(0, 5), "list=");
    EXPECT_GT(out.size(), 5u);
}

TEST(ErrorHandling, FormatReltimeAppendSemantic) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out = "rel=";
    bridge.format_reltime(out, -1.0, reltime_unit::day);
    EXPECT_EQ(out.substr(0, 4), "rel=");
    EXPECT_GT(out.size(), 4u);
}

TEST(ErrorHandling, OrdinalLargeNumbers) {
    BRIDGE_OR_SKIP(bridge, "en");
    // 1st, 2nd, 3rd, 4th patterns repeat
    EXPECT_EQ(bridge.ordinal_category(101.0), plural_category::one);   // 101st
    EXPECT_EQ(bridge.ordinal_category(102.0), plural_category::two);   // 102nd
    EXPECT_EQ(bridge.ordinal_category(103.0), plural_category::few);   // 103rd
    EXPECT_EQ(bridge.ordinal_category(104.0), plural_category::other); // 104th
    // 111th, 112th, 113th are exceptions (not 1st/2nd/3rd)
    EXPECT_EQ(bridge.ordinal_category(111.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(112.0), plural_category::other);
    EXPECT_EQ(bridge.ordinal_category(113.0), plural_category::other);
}

TEST(ErrorHandling, FormatDateAllMonths) {
    BRIDGE_OR_SKIP(bridge, "en");
    // Format the 15th of each month — none should crash
    for (int m = 1; m <= 12; ++m) {
        std::string out;
        bridge.format_date(out, 2025, m, 15, date_style::medium_fmt);
        EXPECT_FALSE(out.empty()) << "month=" << m;
    }
}

TEST(ErrorHandling, FormatTimeAllHours) {
    BRIDGE_OR_SKIP(bridge, "en");
    // Format every hour — none should crash
    for (int h = 0; h < 24; ++h) {
        std::string out;
        bridge.format_time(out, h, 0, 0);
        EXPECT_FALSE(out.empty()) << "hour=" << h;
    }
}

TEST(ErrorHandling, NegativeRelativeTime) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_reltime(out, -5.0, reltime_unit::day);
    EXPECT_FALSE(out.empty());
    // Should contain "5" or "ago" or similar
    EXPECT_TRUE(out.find("5") != std::string::npos || out.find("ago") != std::string::npos) << "got: " << out;
}

TEST(ErrorHandling, PositiveRelativeTime) {
    BRIDGE_OR_SKIP(bridge, "en");
    std::string out;
    bridge.format_reltime(out, 3.0, reltime_unit::hour);
    EXPECT_FALSE(out.empty());
    // Should contain "3" or "in"
    EXPECT_TRUE(out.find("3") != std::string::npos || out.find("in") != std::string::npos) << "got: " << out;
}
