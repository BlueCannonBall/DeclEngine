#include "dictionary.hpp"
#include "Polyweb/string.hpp"
#include "words.hpp"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <ostream>
#include <sstream>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unordered_map>

struct CaseInsensitiveComparer {
    bool operator()(const std::string& a, const std::string& b) const {
        return pw::string::iequals(a, b);
    }
};

struct CaseInsensitiveHasher {
    size_t operator()(const std::string& str) const {
        return std::hash<std::string>()(pw::string::to_lower_copy(str));
    }
};

// These dictionary entries are some of my own, and when any are found for a given word, they take precedence over all of Whitaker's entries.
// Some of these exist because I disagree with Whitaker's definitions, and others exist because some of Whitaker's entries are unparseable.
const std::unordered_multimap<std::string, const WordVariant, CaseInsensitiveHasher, CaseInsensitiveComparer> internal_dictionary = {
    {
        "quid",
        {
            .forms = {
                std::make_shared<Pronoun>(1, CASUS_NOMINATIVE, false, GENDER_NEUTER),
                std::make_shared<Pronoun>(1, CASUS_ACCUSATIVE, false, GENDER_NEUTER),
            },
            .english_base = "what",
        },
    },
    {
        "de",
        {
            .forms = {
                std::make_shared<Preposition>(CASUS_ABLATIVE),
            },
            .english_base = "down",
        },
    },
    {
        "de",
        {
            .forms = {
                std::make_shared<Preposition>(CASUS_ABLATIVE),
            },
            .english_base = "about",
        },
    },
    {
        "rapide",
        {
            .forms = {
                std::make_shared<Adjective>(1, CASUS_VOCATIVE, false, GENDER_MASCULINE, DEGREE_POSITIVE),
            },
            .english_base = "rapid",
        },
    },
    {
        "rapide",
        {
            .forms = {
                std::make_shared<Adverb>(DEGREE_POSITIVE),
            },
            .english_base = "rapidly",
        },
    },
};

std::string remove_accents(const std::string& str) {
    icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(str);

    UErrorCode status = U_ZERO_ERROR;
    icu::Transliterator* transliterator = icu::Transliterator::createInstance("NFD; [:Nonspacing Mark:] Remove; NFC", UTRANS_FORWARD, status);
    transliterator->transliterate(unicode_str);

    std::string ret;
    unicode_str.toUTF8String(ret);
    return ret;
}

size_t query_dictionary(const std::string& word, std::vector<WordVariant>& ret) {
    std::string ascii_word = remove_accents(word);
    for (char& c : ascii_word) { // Remove J
        if (c == 'j') {
            c = 'i';
        } else if (c == 'J') {
            c = 'I';
        }
    }

    auto range = internal_dictionary.equal_range(ascii_word);
    if (range.first != range.second) {
        std::transform(range.first, range.second, std::back_inserter(ret), [](const auto& entry) {
            return entry.second;
        });
        return ret.size();
    }

    // Check if word is entirely composed of digits
    bool is_all_digits = true;
    for (char c : ascii_word) {
        if (!isdigit(c)) {
            is_all_digits = false;
        }
    }
    if (is_all_digits) {
        return 0;
    }

    thread_local WhitakersWords words;
    words.out.ignore(std::numeric_limits<std::streamsize>::max(), '>');
    words.in << ascii_word << std::endl;

    bool last_line_empty = false;
    for (WordVariant variant; words.out;) {
        std::string line;
        std::getline(words.out, line, '\n');
        pw::string::trim_right(line);
        std::istringstream ss(line);
        if (line.empty() && last_line_empty) {
            break;
        } else if (((line.empty() || line.front() == '*') && last_line_empty) ||
                   line == "Two words" ||
                   pw::string::ends_with(line, "UNKNOWN")) {
            break;
        } else if (pw::string::ends_with(line, "MORE - hit RETURN/ENTER to continue")) {
            words.in << std::endl;
            continue;
        } else if (line.front() == ' ' || line.front() == '-') {
            continue;
        }
        last_line_empty = line.empty() || line.front() == '*';

        std::string split_word;
        ss >> split_word;
        split_word.erase(std::remove(split_word.begin(), split_word.end(), '.'), split_word.end());
        if (!pw::string::iequals(split_word, ascii_word)) {
            if (!variant.forms.empty() && std::find_if(line.begin(), line.end(), ispunct) != line.end()) {
                ss.seekg(0);
                for (char c; ss.get(c) && !ispunct(c);) {
                    variant.english_base.push_back(c);
                }

                pw::string::trim_right(variant.english_base);
                if (!variant.english_base.empty()) {
                    ret.push_back(std::move(variant));
                }
            }
            continue;
        }

        std::string string_part_of_speech;
        ss >> string_part_of_speech;
        int unknown;
        switch (hash(string_part_of_speech)) {
        case hash("N"):
        case hash("PRON"): {
            Declension declension;
            std::string string_case;
            char char_plurality;
            char char_gender;
            ss >> declension >> unknown >> string_case >> char_plurality >> char_gender;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"):
            case hash("X"): casus = CASUS_NOMINATIVE; break;
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

            if (string_part_of_speech == "N") {
                variant.forms.push_back(std::make_shared<Noun>(declension, casus, plural, gender));
            } else if (string_part_of_speech == "PRON") {
                variant.forms.push_back(std::make_shared<Pronoun>(declension, casus, plural, gender));
            } else {
                throw std::logic_error("Invalid part of speech");
            }
            break;
        }

        case hash("V"): {
            Conjugation conjugation;
            std::string string_tense;
            std::string string_voice_or_mood;
            std::string string_mood;
            Person person;
            char char_plurality;
            ss >> conjugation >> unknown >> string_tense >> string_voice_or_mood;

            // Parse tense
            Tense tense;
            switch (hash(string_tense)) {
            case hash("PRES"):
            case hash("X"): tense = TENSE_PRESENT; break;
            case hash("IMPF"): tense = TENSE_IMPERFECT; break;
            case hash("PERF"): tense = TENSE_PERFECT; break;
            case hash("PLUP"): tense = TENSE_PLUPERFECT; break;
            case hash("FUT"): tense = TENSE_FUTURE; break;
            case hash("FUTP"): tense = TENSE_FUTURE_PERFECT; break;
            default: throw std::runtime_error("Invalid tense");
            }

            // Parse voice
            Voice voice;
            switch (hash(string_voice_or_mood)) {
            case hash("ACTIVE"):
                voice = VOICE_ACTIVE;
                ss >> string_mood >> person >> char_plurality;
                break;

            case hash("PASSIVE"):
                voice = VOICE_PASSIVE;
                ss >> string_mood >> person >> char_plurality;
                break;

            default:
                voice = VOICE_ACTIVE;
                string_mood = std::move(string_voice_or_mood);
                ss >> person >> char_plurality;
                break;
            }

            // Parse mood
            Mood mood;
            switch (hash(string_mood)) {
            case hash("IND"):
            case hash("X"): mood = MOOD_INDICATIVE; break;
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

            variant.forms.push_back(std::make_shared<Verb>(conjugation, tense, voice, mood, person, plural));
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
            case hash("NOM"):
            case hash("X"): casus = CASUS_NOMINATIVE; break;
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
            case hash("ACTIVE"):
            default: voice = VOICE_ACTIVE; break;
            case hash("PASSIVE"): voice = VOICE_PASSIVE; break;
            }

            variant.forms.push_back(std::make_shared<Participle>(conjugation, casus, plural, gender, tense, voice));
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
            case hash("NOM"):
            case hash("X"): casus = CASUS_NOMINATIVE; break;
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

            variant.forms.push_back(std::make_shared<Supine>(conjugation, casus, plural, gender));
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
            case hash("NOM"):
            case hash("X"): casus = CASUS_NOMINATIVE; break;
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

            variant.forms.push_back(std::make_shared<Adjective>(declension, casus, plural, gender, degree));
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

            variant.forms.push_back(std::make_shared<Adverb>(degree));
            break;
        }

        case hash("CONJ"): {
            variant.forms.push_back(std::make_shared<Conjunction>());
            break;
        }

        case hash("PREP"): {
            std::string string_case;
            ss >> string_case;

            // Parse case
            Casus casus;
            switch (hash(string_case)) {
            case hash("NOM"):
            case hash("X"): casus = CASUS_NOMINATIVE; break;
            case hash("GEN"): casus = CASUS_GENITIVE; break;
            case hash("DAT"): casus = CASUS_DATIVE; break;
            case hash("ACC"): casus = CASUS_ACCUSATIVE; break;
            case hash("ABL"): casus = CASUS_ABLATIVE; break;
            case hash("VOC"): casus = CASUS_VOCATIVE; break;
            case hash("LOC"): casus = CASUS_LOCATIVE; break;
            default: throw std::runtime_error("Invalid case");
            }

            variant.forms.push_back(std::make_shared<Preposition>(casus));
            break;
        }

        case hash("INTERJ"): {
            variant.forms.push_back(std::make_shared<Interjection>());
            break;
        }
        }
    }

    if (ret.empty() && isupper(ascii_word.front())) {
        ret.push_back({
            .forms = {
                std::make_shared<Noun>(0, CASUS_NOMINATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_GENITIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_DATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_ACCUSATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_ABLATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_VOCATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_LOCATIVE, false, GENDER_COMMON),
            },
            .english_base = ascii_word,
        });
    }

    return ret.size();
}
