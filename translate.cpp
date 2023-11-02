#include "translate.hpp"
#include "Polyweb/string.hpp"
#include <algorithm>
#include <boost/process.hpp>
#include <cctype>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

constexpr size_t hash(const char* str, size_t i = 0) {
    return !str[i] ? 5381 : (hash(str, i + 1) * 33) ^ str[i];
}

inline size_t hash(const std::string& str, size_t i = 0) {
    return !str[i] ? 5381 : (hash(str, i + 1) * 33) ^ str[i];
}

std::string Noun::english_equivalent(const std::string& english_base) const {
    static constexpr const char* prefixes[7] = {
        nullptr,
        "of ",
        "to/for ",
        nullptr,
        nullptr,
        "O ",
    };

    std::string ret;
    if (prefixes[casus]) {
        ret = prefixes[casus] + english_base;
    } else {
        ret = english_base;
    }

    if (plural) {
        switch (ret.back()) {
        case 'a':
        case 'i':
        case 'o':
        case 'u':
        case 's':
            ret.push_back('e');
            break;

        case 'y':
            ret.back() = 'i';
            ret.push_back('e');
            break;
        }
        ret.push_back('s');
    }

    return ret;
}

std::string Verb::english_equivalent(const std::string& english_base) const {
    static constexpr const char* be[4][6][3][2] = {
        // Indicative mood
        {
            // Present tense
            {
                {"I am", "we are"},
                {"you are", "y'all are"},
                {"it is", "they are"},
            },
            // Imperfect tense
            {
                {"I was", "we were"},
                {"you were", "y'all were"},
                {"it was", "they were"},
            },
            // Perfect tense
            {
                {"I have been", "we have been"},
                {"you have been", "y'all have been"},
                {"it has been", "they have been"},
            },
            // Pluperfect tense
            {
                {"I had been", "we had been"},
                {"you had been", "y'all had been"},
                {"it had been", "they had been"},
            },
            // Future tense
            {
                {"I shall be", "we shall be"},
                {"you will be", "y'all will be"},
                {"it will be", "they will be"},
            },
            // Future perfect tense
            {
                {"I shall have been", "we shall have been"},
                {"you will have been", "y'all will have been"},
                {"it will have been", "they will have been"},
            },
        },
        // Subjunctive mood
        {
            // Present tense
            {
                {"may I be", "let us be"},
                {"may you be", "may y'all be"},
                {"let it be", "let them be"},
            },
            // Imperfect tense
            {
                {"I should be", "we should be"},
                {"you would be", "y'all would be"},
                {"it would be", "they would be"},
            },
            // Perfect tense
            {
                {"I may have been", "we may have been"},
                {"you may have been", "y'all may have been"},
                {"it may have been", "they may have been"},
            },
            // Pluperfect tense
            {
                {"I should have been", "we should have been"},
                {"you would have been", "y'all would have been"},
                {"it would have been", "they would have been"}},
        },
        {},
        // Infinitive mood
        {
            // Present tense
            {
                {"to be"},
            },
            {},
            // Perfect tense
            {
                {"to have been"},
            },
            {},
            // Future tense
            {
                {"to be about to be"},
            },
        },
    };
    static constexpr const char* prefixes[2][4][6][3][2] = {
        // Active voice
        {
            // Indicative mood
            {
                // Present tense
                {
                    {"I ", "we "},
                    {"you ", "y'all "},
                    {"it ", "they "},
                },
                // Imperfect tense
                {
                    {"I was ", "we were "},
                    {"you were ", "y'all were "},
                    {"it was ", "they were "},
                },
                // Perfect tense
                {
                    {"I ", "we "},
                    {"you ", "y'all "},
                    {"it ", "they "},
                },
                // Pluperfect tense
                {
                    {"I had ", "we had "},
                    {"you had ", "y'all had "},
                    {"it had ", "they had "},
                },
                // Future tense
                {
                    {"I shall ", "we shall "},
                    {"you will ", "y'all will "},
                    {"it will ", "they will "},
                },
                // Future perfect tense
                {
                    {"I shall have ", "we shall have "},
                    {"you will have ", "y'all will have "},
                    {"it will have ", "they will have "},
                },
            },
            // Subjunctive mood
            {
                // Present tense
                {
                    {"may I ", "let us "},
                    {"may you ", "may y'all "},
                    {"let it ", "let them "},
                },
                // Imperfect tense
                {
                    {"I should ", "we should "},
                    {"you should ", "y'all should "},
                    {"it should ", "they should "},
                },
                // Perfect tense
                {
                    {"I may have ", "we may have "},
                    {"you may have ", "y'all may have "},
                    {"it may have ", "they may have "},
                },
                // Pluperfect tense
                {
                    {"I should have ", "we should have "},
                    {"you would have ", "y'all would have "},
                    {"it would have ", "they would have "},
                },
            },
            // Imperative mood
            {
                {},
                {},
                {},
                {},
                // Future tense
                {
                    {},
                    {"you shall ", "y'all shall "},
                    {"it shall ", "they shall "},
                },
            },
            // Infinitive mood
            {
                // Present tense
                {
                    {"to "},
                },
                {},
                // Perfect tense
                {
                    {"to have "},
                },
                {},
                // Future tense
                {
                    {"to be about to "},
                },
            },
        },
        // Passive voice
        {
            // Indicative mood
            {
                // Present tense
                {
                    {"I am ", "we are "},
                    {"you are ", "y'all are "},
                    {"it is ", "they are "},
                },
                // Imperfect tense
                {
                    {"I was ", "we were "},
                    {"you were ", "y'all were "},
                    {"it was ", "they were "},
                },
                {},
                {},
                // Future tense
                {
                    {"I shall be ", "we will be "},
                    {"you will be ", "y'all will be "},
                    {"it will be ", "they will be "},
                },
            },
            // Subjunctive mood
            {
                // Present tense
                {
                    {"may I be ", "may we be "},
                    {"may you be ", "may y'all be "},
                    {"let it be ", "let them be "},
                },
                // Imperfect tense
                {
                    {"I should be ", "we should be "},
                    {"you would be ", "y'all would be "},
                    {"it would be ", "they would be "},
                },
            },
            // Imperative mood
            {
                // Present tense
                {
                    {},
                    {"be ", "be "},
                },
                {},
                {},
                {},
                // Future tense
                {
                    {},
                    {"you shall be "},
                    {"it shall be ", "they shall be "},
                },
            },
            // Infinitive mood
            {
                // Present tense
                {
                    {"to be "},
                },
            },
        },
    };

    if (english_base == "be") {
        if (be[mood][tense][person][plural]) {
            return be[mood][tense][person][plural];
        }
    }

    std::string ret;
    if (prefixes[voice][mood][tense][person][plural]) {
        ret = prefixes[voice][mood][tense][person][plural] + english_base; // Add prefix
    } else {
        ret = english_base;
    }

    // Add suffix
    switch (voice) {
    case VOICE_ACTIVE:
        switch (mood) {
        case MOOD_INDICATIVE:
            switch (tense) {
            case TENSE_IMPERFECT:
                if (ret.back() == 'e') {
                    ret.back() = 'i';
                    ret += "ng";
                } else {
                    ret += "ing";
                }
                break;

            case TENSE_PERFECT:
            case TENSE_PLUPERFECT:
            case TENSE_FUTURE_PERFECT:
                if (ret.back() == 'e') {
                    ret.push_back('d');
                } else {
                    ret += "ed";
                }
                break;

            default:
                break;
            }
            break;

        case MOOD_SUBJUNCTIVE:
            switch (tense) {
            case TENSE_PERFECT:
            case TENSE_PLUPERFECT:
            case TENSE_FUTURE_PERFECT:
                if (ret.back() == 'e') {
                    ret.push_back('d');
                } else {
                    ret += "ed";
                }
                break;

            default:
                break;
            }
            break;

        case MOOD_INFINITIVE:
            if (tense == TENSE_PERFECT) {
                if (ret.back() == 'e') {
                    ret.push_back('d');
                } else {
                    ret += "ed";
                }
            }
            break;

        default:
            break;
        }
        break;

    case VOICE_PASSIVE:
        if (ret.back() == 'e') {
            ret.push_back('d');
        } else {
            ret += "ed";
        }
        break;
    }

    return ret;
}

std::string Participle::english_equivalent(const std::string& english_base) const {
    static constexpr const char* prefixes[7] = {
        nullptr,
        "of ",
        "to/for ",
        nullptr,
        "by ",
    };

    std::string ret;
    if (prefixes[casus]) {
        ret = prefixes[casus] + english_base;
    } else {
        ret = english_base;
    }

    if (ret.back() == 'e') {
        ret.back() = 'i';
        ret += "ng";
    } else {
        ret += "ing";
    }

    return ret;
}

std::string Supine::english_equivalent(const std::string& english_base) const {
    switch (casus) {
    case CASUS_ACCUSATIVE: return "to " + english_base;
    case CASUS_ABLATIVE: return "be " + english_base;
    default: throw std::logic_error("Invalid case");
    }
}

std::string Adjective::english_equivalent(const std::string& english_base) const {
    if (degree) {
        switch (degree) {
        case DEGREE_COMPARATIVE:
            if (english_base.back() == 'e') {
                return english_base + 'r';
            } else {
                return english_base + "er";
            }

        case DEGREE_SUPERLATIVE:
            return "most-" + english_base;

        default:
            throw std::logic_error("Invalid degree of comparison");
        }
    } else if (plural) {
        std::string ret;
        switch (english_base.back()) {
        case 'y':
            ret.back() = 'i';
        case 'a':
        case 'i':
        case 'o':
        case 'u':
        case 's':
            ret.push_back('e');
            break;
        }
        ret.push_back('s');
        return ret;
    }

    return english_base;
}

std::string Adverb::english_equivalent(const std::string& english_base) const {
    if (degree) {
        std::string ret;
        if (pw::string::ends_with(english_base, "ly")) {
            ret = english_base.substr(0, english_base.size() - 2);
        }

        switch (degree) {
        case DEGREE_COMPARATIVE:
            if (ret.back() == 'e') {
                ret.push_back('r');
            } else if (ret.back() == 'i') {
                ret.back() = 'e';
                ret.push_back('r');
            } else {
                ret += "er";
            }
            break;

        case DEGREE_SUPERLATIVE:
            ret.insert(0, "most-");
            break;

        default:
            throw std::logic_error("Invalid degree of comparison");
        }

        return ret;
    } else if (!pw::string::ends_with(english_base, "ly")) {
        if (english_base.back() == 'y') {
            return english_base.substr(0, english_base.size() - 1) + "ily";
        } else {
            return english_base + "ly";
        }
    }

    return english_base;
}

std::string remove_accents(const std::string& str) {
    icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(str);

    UErrorCode status = U_ZERO_ERROR;
    icu::Transliterator* transliterator = icu::Transliterator::createInstance("NFD; [:Nonspacing Mark:] Remove; NFC", UTRANS_FORWARD, status);
    transliterator->transliterate(unicode_str);

    std::string ret;
    unicode_str.toUTF8String(ret);
    return ret;
}

WordInfo query_whitakers_words(const std::string& word) {
    std::string ascii_word = remove_accents(word);

    boost::process::ipstream out;
    boost::process::child words("bin/words", ascii_word, boost::process::start_dir("whitakers-words"), boost::process::std_out > out);

    WordInfo ret;

    for (; out;) {
        std::string line;
        std::getline(out, line, '\n');
        pw::string::trim_right(line);
        std::istringstream ss(line);
        if (line == "Two words" ||
            pw::string::ends_with(line, "UNKNOWN")) {
            break;
        } else if (line.front() == ' ') {
            continue;
        }

        std::string split_word;
        ss >> split_word;
        split_word.erase(std::remove_if(split_word.begin(), split_word.end(), ispunct), split_word.end());
        if (!ret.variants.empty() && split_word != ascii_word) { // Variants have ended and definitions have begun
            ss.seekg(0);
            for (char c; ss.get(c) && !ispunct(c);) {
                ret.english_base.push_back(c);
            }
            pw::string::trim_right(ret.english_base);
            break;
        }

        std::string string_part_of_speech;
        ss >> string_part_of_speech;
        int unknown;
        switch (hash(string_part_of_speech)) {
        case hash("N"): {
            Declension declension;
            std::string string_case;
            char char_plurality;
            char char_gender;
            ss >> declension >> unknown >> string_case >> char_plurality >> char_gender;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"): casus = CASUS_NOMINATIVE; break;
            case hash("GEN"): casus = CASUS_GENITIVE; break;
            case hash("DAT"): casus = CASUS_DATIVE; break;
            case hash("ACC"): casus = CASUS_ACCUSATIVE; break;
            case hash("ABL"): casus = CASUS_ABLATIVE; break;
            case hash("VOC"): casus = CASUS_VOCATIVE; break;
            case hash("LOC"): casus = CASUS_LOCATIVE; break;
            default: throw std::runtime_error("Invalid case");
            }

            // Parse plurality
            bool plural = char_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (char_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
            case 'C':
            case 'X': gender = GENDER_COMMON; break;
            default: throw std::runtime_error("Invalid gender");
            }

            ret.variants.push_back(std::make_unique<Noun>(declension, casus, plural, gender));
            break;
        }

        case hash("V"): {
            Conjugation conjugation;
            std::string string_tense;
            std::string string_voice;
            std::string string_mood;
            Person person;
            char char_plurality;
            ss >> conjugation >> unknown >> string_tense >> string_voice >> string_mood >> person >> char_plurality;

            // Parse tense
            Tense tense;
            switch (hash(string_tense)) {
            case hash("PRES"): tense = TENSE_PRESENT; break;
            case hash("IMPF"): tense = TENSE_IMPERFECT; break;
            case hash("PERF"): tense = TENSE_PERFECT; break;
            case hash("PLUP"): tense = TENSE_PLUPERFECT; break;
            case hash("FUT"): tense = TENSE_FUTURE; break;
            case hash("FUTP"): tense = TENSE_FUTURE_PERFECT; break;
            default: throw std::runtime_error("Invalid tense");
            }

            // Parse voice
            Voice voice;
            switch (hash(string_voice)) {
            case hash("ACTIVE"): voice = VOICE_ACTIVE; break;
            case hash("PASSIVE"): voice = VOICE_PASSIVE; break;
            default: throw std::runtime_error("Invalid voice");
            }

            // Parse mood
            Mood mood;
            switch (hash(string_mood)) {
            case hash("IND"): mood = MOOD_INDICATIVE; break;
            case hash("SUB"): mood = MOOD_SUBJUNCTIVE; break;
            case hash("IMP"): mood = MOOD_IMPERATIVE; break;
            case hash("INF"): mood = MOOD_INFINITIVE; break;
            default: throw std::runtime_error("Invalid mood");
            }

            // Limit person
            if (person > 0) {
                --person;
            }

            // Parse plurality
            bool plural = char_plurality == 'P';

            ret.variants.push_back(std::make_unique<Verb>(conjugation, tense, voice, mood, person, plural));
            break;
        }

        case hash("VPAR"): {
            Conjugation conjugation;
            std::string string_case;
            char char_plurality;
            char char_gender;
            std::string string_tense;
            std::string string_voice;
            ss >> conjugation >> unknown >> string_case >> char_plurality >> char_gender >> string_tense >> string_voice;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"): casus = CASUS_NOMINATIVE; break;
            case hash("GEN"): casus = CASUS_GENITIVE; break;
            case hash("DAT"): casus = CASUS_DATIVE; break;
            case hash("ACC"): casus = CASUS_ACCUSATIVE; break;
            case hash("ABL"): casus = CASUS_ABLATIVE; break;
            case hash("VOC"): casus = CASUS_VOCATIVE; break;
            case hash("LOC"): casus = CASUS_LOCATIVE; break;
            default: throw std::runtime_error("Invalid case");
            }

            // Parse plurality
            bool plural = char_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (char_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
            case 'C':
            case 'X': gender = GENDER_COMMON; break;
            default: throw std::runtime_error("Invalid gender");
            }

            // Parse tense
            Tense tense;
            switch (hash(string_tense)) {
            case hash("PRES"): tense = TENSE_PRESENT; break;
            case hash("IMPF"): tense = TENSE_IMPERFECT; break;
            case hash("PERF"): tense = TENSE_PERFECT; break;
            case hash("PLUP"): tense = TENSE_PLUPERFECT; break;
            case hash("FUT"): tense = TENSE_FUTURE; break;
            case hash("FUTP"): tense = TENSE_FUTURE_PERFECT; break;
            default: throw std::runtime_error("Invalid tense");
            }

            // Parse voice
            Voice voice;
            switch (hash(string_voice)) {
            case hash("ACTIVE"): voice = VOICE_ACTIVE; break;
            case hash("PASSIVE"): voice = VOICE_PASSIVE; break;
            default: throw std::runtime_error("Invalid voice");
            }

            ret.variants.push_back(std::make_unique<Participle>(conjugation, casus, plural, gender, tense, voice));
            break;
        }

        case hash("SUPINE"): {
            Conjugation conjugation;
            std::string string_case;
            char char_plurality;
            char char_gender;
            ss >> conjugation >> unknown >> string_case >> char_plurality >> char_gender;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"): casus = CASUS_NOMINATIVE; break;
            case hash("GEN"): casus = CASUS_GENITIVE; break;
            case hash("DAT"): casus = CASUS_DATIVE; break;
            case hash("ACC"): casus = CASUS_ACCUSATIVE; break;
            case hash("ABL"): casus = CASUS_ABLATIVE; break;
            case hash("VOC"): casus = CASUS_VOCATIVE; break;
            case hash("LOC"): casus = CASUS_LOCATIVE; break;
            default: throw std::runtime_error("Invalid case");
            }

            // Parse plurality
            bool plural = char_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (char_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
            case 'C':
            case 'X': gender = GENDER_COMMON; break;
            default: throw std::runtime_error("Invalid gender");
            }

            ret.variants.push_back(std::make_unique<Supine>(conjugation, casus, plural, gender));
            break;
        }

        case hash("ADJ"): {
            Declension declension;
            std::string string_case;
            char char_plurality;
            char char_gender;
            std::string string_degree;
            ss >> declension >> unknown >> string_case >> char_plurality >> char_gender >> string_degree;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"): casus = CASUS_NOMINATIVE; break;
            case hash("GEN"): casus = CASUS_GENITIVE; break;
            case hash("DAT"): casus = CASUS_DATIVE; break;
            case hash("ACC"): casus = CASUS_ACCUSATIVE; break;
            case hash("ABL"): casus = CASUS_ABLATIVE; break;
            case hash("VOC"): casus = CASUS_VOCATIVE; break;
            case hash("LOC"): casus = CASUS_LOCATIVE; break;
            default: throw std::runtime_error("Invalid case");
            }

            // Parse plurality
            bool plural = char_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (char_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
            case 'C':
            case 'X': gender = GENDER_COMMON; break;
            default: throw std::runtime_error("Invalid gender");
            }

            // Parse degree
            Degree degree;
            switch (hash(string_degree)) {
            case hash("POS"): degree = DEGREE_POSITIVE; break;
            case hash("COMP"): degree = DEGREE_COMPARATIVE; break;
            case hash("SUPER"): degree = DEGREE_SUPERLATIVE; break;
            default: throw std::runtime_error("Invalid degree of comparison");
            }

            ret.variants.push_back(std::make_unique<Adjective>(declension, casus, plural, gender, degree));
            break;
        }

        case hash("ADV"): {
            std::string string_degree;
            ss >> string_degree;

            // Parse degree
            Degree degree;
            switch (hash(string_degree)) {
            case hash("POS"): degree = DEGREE_POSITIVE; break;
            case hash("COMP"): degree = DEGREE_COMPARATIVE; break;
            case hash("SUPER"): degree = DEGREE_SUPERLATIVE; break;
            default: throw std::runtime_error("Invalid degree of comparison");
            }

            ret.variants.push_back(std::make_unique<Adverb>(degree));
            break;
        }
        }
    }

    return ret;
}
