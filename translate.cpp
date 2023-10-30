#include "translate.hpp"
#include "Polyweb/string.hpp"
#include <boost/process.hpp>
#include <cctype>
#include <memory>
#include <iostream>
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
    std::string ret = english_base;

    if (casus == CASUS_GENITIVE) {
        ret.insert(0, "of ");
    } else if (casus == CASUS_DATIVE) {
        ret.insert(0, "to/for ");
    }

    if (plural) {
        switch (ret.back()) {
        case 'a':
        case 'i':
        case 'o':
        case 'u':
            ret.push_back('e');
            break;

        case 'y':
            ret.pop_back();
            ret += "ie";
            break;
        }
        ret.push_back('s');
    }

    return ret;
}

std::string Verb::english_equivalent(const std::string& english_base) const {
    std::string ret = english_base;

    switch (number) {
    case 1:
        if (tense == TENSE_IMPERFECT) {
            ret.insert(0, plural ? "were " : "was ");
        }
        ret.insert(0, plural ? "we " : "I ");
        break;

    case 2:
        if (tense == TENSE_IMPERFECT) {
            ret.insert(0, "were ");
        }
        ret.insert(0, plural ? "y'all " : "you ");
        break;

    case 3:
        if (tense == TENSE_IMPERFECT) {
            ret.insert(0, plural ? "were " : "was ");
        }
        ret.insert(0, plural ? "they " : "it ");
        break;
    }

    switch (tense) {
    case TENSE_IMPERFECT:
        if (ret.back() == 'e') {
            ret.pop_back();
        }
        ret += "ing";
        break;

    case TENSE_PERFECT:
        if (ret.back() == 'e') {
            ret.push_back('d');
        } else {
            ret += "ed";
        }
        break;

    case TENSE_PLUPERFECT:
        ret.insert(0, "had ");
        if (ret.back() == 'e') {
            ret.push_back('d');
        } else {
            ret += "ed";
        }
        break;

    default:
        break;
    }

    return ret;
}

std::string Adjective::english_equivalent(const std::string& english_base) const {
    std::string ret = english_base;

    if (plural) {
        switch (ret.back()) {
        case 'a':
        case 'i':
        case 'o':
        case 'u':
            ret.push_back('e');
            break;

        case 'y':
            ret.pop_back();
            ret += "ie";
            break;
        }
        ret.push_back('s');
    }

    return ret;
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
        pw::string::trim(line);
        if (line == "Two words" ||
            pw::string::ends_with(line, "UNKNOWN")) {
            break;
        } else if (line.find('[') != std::string::npos ||
            line.find(']') != std::string::npos) {
            continue;
        }
        std::istringstream ss(line);

        std::string split_word;
        ss >> split_word;
        if (!ret.variants.empty() && split_word != ret.split_word) { // Variants have ended and definitions have begun
            ss.seekg(0);
            for (char c; ss.get(c) && !ispunct(c);) {
                ret.english_base.push_back(c);
            }
            pw::string::trim_right(ret.english_base);
            break;
        }
        ret.split_word = split_word;

        std::string string_part_of_speech;
        ss >> string_part_of_speech;
        int unknown;
        switch (hash(string_part_of_speech)) {
        case hash("N"): {
            Declension declension;
            std::string string_case;
            char string_plurality;
            char string_gender;
            ss >> declension >> unknown >> string_case >> string_plurality >> string_gender;

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
            bool plural = string_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (string_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
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
            Number number;
            char plurality;
            ss >> conjugation >> unknown >> string_tense >> string_voice >> string_mood >> number >> plurality;

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
            case hash("INF"): mood = MOOD_INFINITIVE; break;
            case hash("IMP"): mood = MOOD_IMPERATIVE; break;
            case hash("SUB"): mood = MOOD_SUBJUNCTIVE; break;
            default: throw std::runtime_error("Invalid mood");
            }

            // Parse plurality
            bool plural = plurality == 'P';

            ret.variants.push_back(std::make_unique<Verb>(conjugation, tense, voice, mood, number, plural));
            break;
        }

        case hash("ADJ"): {
            Declension declension;
            std::string string_case;
            char string_plurality;
            char string_gender;
            ss >> declension >> unknown >> string_case >> string_plurality >> string_gender;

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
            bool plural = string_plurality == 'P';

            // Parse gender
            Gender gender;
            switch (string_gender) {
            case 'M': gender = GENDER_MASCULINE; break;
            case 'F': gender = GENDER_FEMININE; break;
            case 'N': gender = GENDER_NEUTER; break;
            default: throw std::runtime_error("Invalid gender");
            }

            ret.variants.push_back(std::make_unique<Adjective>(declension, casus, plural, gender));
            break;
        }
        }
    }

    return ret;
}
