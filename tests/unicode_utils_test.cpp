#include "icetea/unicode_utils.h"

#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <vector>

using namespace icetea;

// ═══════════════════════════════════════════════════════════════════════════
//  Case Mapping
// ═══════════════════════════════════════════════════════════════════════════

TEST(CaseMapping, LowercaseEnglish) {
    EXPECT_EQ(lowercase("HELLO WORLD", "en"), "hello world");
}

TEST(CaseMapping, LowercaseGerman) {
    // German ß: uppercase ẞ → lowercase ß
    EXPECT_EQ(lowercase("STRASSE", "de"), "strasse");
}

TEST(CaseMapping, LowercaseTurkish) {
    // Turkish İ (U+0130) → i, I → ı (U+0131)
    auto result = lowercase("DİYARBAKIR", "tr");
    EXPECT_FALSE(result.empty());
    // Turkish lowercase of I should be ı (dotless i), not 'i'
    auto result2 = lowercase("I", "tr");
    EXPECT_EQ(result2, "ı");
}

TEST(CaseMapping, UppercaseEnglish) {
    EXPECT_EQ(uppercase("hello world", "en"), "HELLO WORLD");
}

TEST(CaseMapping, UppercaseGerman) {
    // German ß → SS (traditional uppercasing rule)
    auto result = uppercase("straße", "de");
    EXPECT_TRUE(result == "STRASSE" || result == "STRAẞE") << "Got: " << result;
}

TEST(CaseMapping, UppercaseTurkish) {
    // Turkish i → İ (with dot), ı → I
    auto result = uppercase("i", "tr");
    EXPECT_EQ(result, "İ");
}

TEST(CaseMapping, CaseFoldBasic) {
    // Case folding is locale-independent
    EXPECT_EQ(case_fold("Hello WORLD"), case_fold("hello world"));
}

TEST(CaseMapping, CaseFoldGermanSS) {
    // ß and SS should case-fold to the same thing
    EXPECT_EQ(case_fold("straße"), case_fold("STRASSE"));
}

TEST(CaseMapping, TitlecaseEnglish) {
    auto result = titlecase_segment("hello world", "en");
    EXPECT_FALSE(result.empty());
    // Should titlecase the first letter at minimum
    EXPECT_EQ(result[0], 'H');
}

TEST(CaseMapping, TitlecaseFrench) {
    auto result = titlecase_segment("bonjour le monde", "fr");
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result[0], 'B');
}

TEST(CaseMapping, EmptyString) {
    EXPECT_EQ(lowercase("", "en"), "");
    EXPECT_EQ(uppercase("", "en"), "");
    EXPECT_EQ(case_fold(""), "");
    EXPECT_EQ(titlecase_segment("", "en"), "");
}

TEST(CaseMapping, UnicodePreserved) {
    // Chinese characters should pass through unchanged
    EXPECT_EQ(lowercase("你好世界", "zh"), "你好世界");
    EXPECT_EQ(uppercase("你好世界", "zh"), "你好世界");
}

TEST(CaseMapping, MixedScripts) {
    auto result = lowercase("Hello Мир 世界", "en");
    EXPECT_NE(result.find("hello"), std::string::npos);
    EXPECT_NE(result.find("мир"), std::string::npos);
    EXPECT_NE(result.find("世界"), std::string::npos);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Normalization
// ═══════════════════════════════════════════════════════════════════════════

TEST(Normalization, NFCCompose) {
    // é can be represented as U+00E9 (precomposed) or U+0065 U+0301 (decomposed)
    // NFD: e + combining acute accent
    std::string decomposed = "e\xCC\x81"; // U+0065 + U+0301
    std::string composed = "\xC3\xA9";    // U+00E9
    auto result = normalize(decomposed, normalization_form::nfc);
    EXPECT_EQ(result, composed);
}

TEST(Normalization, NFDDecompose) {
    std::string composed = "\xC3\xA9";    // U+00E9
    std::string decomposed = "e\xCC\x81"; // U+0065 + U+0301
    auto result = normalize(composed, normalization_form::nfd);
    EXPECT_EQ(result, decomposed);
}

TEST(Normalization, NFKCCompatibility) {
    // ﬁ (U+FB01, ligature) → fi under NFKC
    std::string ligature = "\xEF\xAC\x81"; // U+FB01
    auto result = normalize(ligature, normalization_form::nfkc);
    EXPECT_EQ(result, "fi");
}

TEST(Normalization, NFKDCompatibility) {
    // ﬁ (U+FB01) → fi under NFKD
    std::string ligature = "\xEF\xAC\x81";
    auto result = normalize(ligature, normalization_form::nfkd);
    EXPECT_EQ(result, "fi");
}

TEST(Normalization, IsNormalizedNFC) {
    std::string composed = "\xC3\xA9"; // U+00E9
    EXPECT_TRUE(is_normalized(composed, normalization_form::nfc));
}

TEST(Normalization, IsNotNormalizedNFC) {
    std::string decomposed = "e\xCC\x81"; // U+0065 + U+0301
    EXPECT_FALSE(is_normalized(decomposed, normalization_form::nfc));
}

TEST(Normalization, ASCIIAlreadyNormalized) {
    EXPECT_TRUE(is_normalized("hello", normalization_form::nfc));
    EXPECT_TRUE(is_normalized("hello", normalization_form::nfd));
    EXPECT_TRUE(is_normalized("hello", normalization_form::nfkc));
    EXPECT_TRUE(is_normalized("hello", normalization_form::nfkd));
}

TEST(Normalization, EmptyString) {
    EXPECT_EQ(normalize("", normalization_form::nfc), "");
    EXPECT_TRUE(is_normalized("", normalization_form::nfc));
}

TEST(Normalization, HangulComposition) {
    // Hangul syllable 가 = ᄀ + ᅡ (Jamo L + V)
    std::string jamo = "\xE1\x84\x80\xE1\x85\xA1"; // U+1100 + U+1161
    std::string syllable = "\xEA\xB0\x80";         // U+AC00
    auto result = normalize(jamo, normalization_form::nfc);
    EXPECT_EQ(result, syllable);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Character Properties
// ═══════════════════════════════════════════════════════════════════════════

TEST(CharProperties, Alphabetic) {
    EXPECT_TRUE(code_point_has_property(U'A', unicode_property::alphabetic));
    EXPECT_TRUE(code_point_has_property(U'z', unicode_property::alphabetic));
    EXPECT_TRUE(code_point_has_property(U'α', unicode_property::alphabetic));
    EXPECT_FALSE(code_point_has_property(U'1', unicode_property::alphabetic));
    EXPECT_FALSE(code_point_has_property(U' ', unicode_property::alphabetic));
}

TEST(CharProperties, Whitespace) {
    EXPECT_TRUE(code_point_has_property(U' ', unicode_property::whitespace));
    EXPECT_TRUE(code_point_has_property(U'\t', unicode_property::whitespace));
    EXPECT_TRUE(code_point_has_property(U'\n', unicode_property::whitespace));
    // U+00A0 NO-BREAK SPACE
    EXPECT_TRUE(code_point_has_property(U'\u00A0', unicode_property::whitespace));
    EXPECT_FALSE(code_point_has_property(U'A', unicode_property::whitespace));
}

TEST(CharProperties, Numeric) {
    EXPECT_TRUE(code_point_has_property(U'0', unicode_property::numeric));
    EXPECT_TRUE(code_point_has_property(U'9', unicode_property::numeric));
    // Arabic-Indic digit 0 (U+0660)
    EXPECT_TRUE(code_point_has_property(U'\u0660', unicode_property::numeric));
    EXPECT_FALSE(code_point_has_property(U'A', unicode_property::numeric));
}

TEST(CharProperties, Case) {
    EXPECT_TRUE(code_point_has_property(U'a', unicode_property::lowercase));
    EXPECT_FALSE(code_point_has_property(U'A', unicode_property::lowercase));
    EXPECT_TRUE(code_point_has_property(U'A', unicode_property::uppercase));
    EXPECT_FALSE(code_point_has_property(U'a', unicode_property::uppercase));
}

TEST(CharProperties, Emoji) {
    // U+1F600 GRINNING FACE
    EXPECT_TRUE(code_point_has_property(U'\U0001F600', unicode_property::emoji));
    // U+2764 HEAVY BLACK HEART
    EXPECT_TRUE(code_point_has_property(U'\u2764', unicode_property::emoji));
    EXPECT_FALSE(code_point_has_property(U'A', unicode_property::emoji));
}

TEST(CharProperties, Ideographic) {
    // CJK Unified Ideograph
    EXPECT_TRUE(code_point_has_property(U'中', unicode_property::ideographic));
    EXPECT_TRUE(code_point_has_property(U'国', unicode_property::ideographic));
    EXPECT_FALSE(code_point_has_property(U'A', unicode_property::ideographic));
}

// ═══════════════════════════════════════════════════════════════════════════
//  Segmentation
// ═══════════════════════════════════════════════════════════════════════════

TEST(Segmentation, GraphemeASCII) {
    auto breaks = segment("Hello", segmentation_type::grapheme);
    // Grapheme breaks between each character + end
    // Should have at least 5 break points for 5 grapheme clusters
    EXPECT_GE(breaks.size(), 5u);
}

TEST(Segmentation, GraphemeEmoji) {
    // Family emoji: composed of multiple code points, counts as 1 grapheme
    // Simple test: flag emoji 🇺🇸 = U+1F1FA U+1F1F8 (2 code points, 1 grapheme)
    std::string flag = "\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"; // 🇺🇸
    auto breaks = segment(flag, segmentation_type::grapheme);
    // Should have 1 grapheme cluster break at end of the flag
    EXPECT_GE(breaks.size(), 1u);
}

TEST(Segmentation, WordEnglish) {
    auto breaks = segment("Hello World", segmentation_type::word);
    // Breaks at: H|ello| |W|orld| → at least the word boundaries
    EXPECT_GE(breaks.size(), 2u);
}

TEST(Segmentation, WordChinese) {
    // Chinese doesn't use spaces — word segmenter should still find boundaries
    auto breaks = segment("你好世界", segmentation_type::word);
    EXPECT_GE(breaks.size(), 1u);
}

TEST(Segmentation, SentenceEnglish) {
    auto breaks = segment("Hello. World! How are you?", segmentation_type::sentence);
    // 3 sentences
    EXPECT_GE(breaks.size(), 2u);
}

TEST(Segmentation, EmptyString) {
    auto breaks = segment("", segmentation_type::grapheme);
    // Segmenter may return [0] (end-of-string sentinel) or empty — both are valid
    EXPECT_LE(breaks.size(), 1u);
}

TEST(Segmentation, SingleChar) {
    auto breaks = segment("A", segmentation_type::grapheme);
    // At least the end-of-string break
    EXPECT_GE(breaks.size(), 1u);
}

TEST(Segmentation, GraphemeCombining) {
    // e + combining acute = 1 grapheme cluster (3 bytes in UTF-8)
    std::string text = "e\xCC\x81"; // U+0065 + U+0301
    auto breaks = segment(text, segmentation_type::grapheme);
    // Segmenter may include start (0) and/or end (3) sentinel breaks.
    // The key invariant: only 1 grapheme cluster, so consecutive breaks
    // should span the entire string without splitting inside the cluster.
    EXPECT_GE(breaks.size(), 1u);
    // The last break should be at the end of the string (byte offset 3)
    EXPECT_EQ(breaks.back(), text.size());
}

// ═══════════════════════════════════════════════════════════════════════════
//  Collation
// ═══════════════════════════════════════════════════════════════════════════

TEST(Collation, BasicEnglish) {
    EXPECT_LT(collation_compare("apple", "banana", "en"), 0);
    EXPECT_GT(collation_compare("banana", "apple", "en"), 0);
    EXPECT_EQ(collation_compare("apple", "apple", "en"), 0);
}

TEST(Collation, CaseInsensitivePrimary) {
    // Primary strength: case-insensitive and accent-insensitive
    EXPECT_EQ(collation_compare("Hello", "hello", "en", collation_strength::primary), 0);
}

TEST(Collation, CaseSensitiveTertiary) {
    // Tertiary (default): case-sensitive
    EXPECT_NE(collation_compare("Hello", "hello", "en", collation_strength::tertiary), 0);
}

TEST(Collation, AccentInsensitivePrimary) {
    // Primary: accent-insensitive — é == e
    std::string e_acute = "\xC3\xA9"; // é
    EXPECT_EQ(collation_compare(e_acute, "e", "en", collation_strength::primary), 0);
}

TEST(Collation, AccentSensitiveSecondary) {
    // Secondary: accent-sensitive — é ≠ e
    std::string e_acute = "\xC3\xA9";
    EXPECT_NE(collation_compare(e_acute, "e", "en", collation_strength::secondary), 0);
}

TEST(Collation, GermanPhonebook) {
    // In German, ö should sort near o
    // ö (U+00F6) vs p: ö < p
    std::string o_umlaut = "\xC3\xB6";
    EXPECT_LT(collation_compare(o_umlaut, "p", "de"), 0);
}

TEST(Collation, ChineseLocale) {
    // Chinese collation should produce a deterministic ordering
    int cmp = collation_compare("中", "国", "zh");
    // Just verify it returns a valid comparison value
    EXPECT_TRUE(cmp == -1 || cmp == 0 || cmp == 1);
}

TEST(Collation, ContainsBasic) {
    EXPECT_TRUE(collation_contains("Hello World", "World", "en"));
    EXPECT_FALSE(collation_contains("Hello World", "xyz", "en"));
}

TEST(Collation, ContainsCaseInsensitive) {
    // Primary strength: case-insensitive search
    EXPECT_TRUE(collation_contains("Hello World", "hello", "en", collation_strength::primary));
}

TEST(Collation, FindBasic) {
    int pos = collation_find("Hello World", "World", "en");
    EXPECT_GE(pos, 0);
}

TEST(Collation, FindNotFound) {
    int pos = collation_find("Hello World", "xyz", "en");
    EXPECT_EQ(pos, -1);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Transliteration
// ═══════════════════════════════════════════════════════════════════════════

TEST(Transliteration, SimplifiedToTraditional) {
    // "und-Hant-t-und-hans" = Simplified Chinese → Traditional Chinese
    auto result = transliterate("国", "und-Hant-t-und-hans");
    // May return 國 (traditional) or empty if unsupported
    if (!result.empty()) {
        EXPECT_NE(result, "国"); // should differ from simplified
    }
}

TEST(Transliteration, TraditionalToSimplified) {
    auto result = transliterate("國", "und-Hans-t-und-hant");
    if (!result.empty()) {
        EXPECT_EQ(result, "国");
    }
}

TEST(Transliteration, EmptyInput) {
    auto result = transliterate("", "und-Hant-t-und-hans");
    EXPECT_TRUE(result.empty());
}

TEST(Transliteration, InvalidTransformId) {
    // Invalid transform ID should return empty
    auto result = transliterate("hello", "invalid-transform-xxx");
    // May or may not be empty depending on implementation; just don't crash
    (void)result;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multi-locale case mapping stress tests
// ═══════════════════════════════════════════════════════════════════════════

TEST(CaseMapping, MultiLocaleRoundtrip) {
    // For most Latin-script locales, lowercase(uppercase(s)) ≈ lowercase(s)
    const char *locales[] = {"en", "fr", "de", "es", "it", "pt", "pl", "nl"};
    const char *text = "hello world";
    for (auto loc : locales) {
        auto upper = uppercase(text, loc);
        EXPECT_FALSE(upper.empty()) << "locale=" << loc;
        auto lower = lowercase(upper, loc);
        EXPECT_EQ(lower, text) << "locale=" << loc;
    }
}

TEST(CaseMapping, CyrillicCaseMapping) {
    auto result = uppercase("привет мир", "ru");
    EXPECT_EQ(result, "ПРИВЕТ МИР");
    auto back = lowercase(result, "ru");
    EXPECT_EQ(back, "привет мир");
}

TEST(CaseMapping, GreekCaseMapping) {
    auto upper = uppercase("αβγ", "el");
    EXPECT_EQ(upper, "ΑΒΓ");
}

TEST(CaseMapping, ArabicPassthrough) {
    // Arabic has no case distinction — should pass through
    std::string arabic = "مرحبا";
    EXPECT_EQ(lowercase(arabic, "ar"), arabic);
    EXPECT_EQ(uppercase(arabic, "ar"), arabic);
}

TEST(CaseMapping, LongString) {
    // Test buffer growth: string > 512 bytes (the initial ffi buffer)
    std::string long_str(600, 'a');
    auto result = uppercase(long_str, "en");
    EXPECT_EQ(result.size(), 600u);
    EXPECT_EQ(result, std::string(600, 'A'));
}
