#include "Polyweb/string.hpp"
#include "json.hpp"
#include "words.hpp"
#include <ctype.h>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string.h>
#include <string_view>
#include <unordered_map>
#include <utility>

using nlohmann::json;

thread_local std::unordered_map<std::string, std::pair<std::string, std::string>> irregular_verbs;

bool is_vowel(char c, bool include_y = false) {
    c = tolower(c);
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || (include_y && c == 'y');
}

bool is_consonant(char c, bool include_y = true) {
    c = tolower(c);
    return c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' && (include_y || c != 'y');
}

bool is_short(std::string_view word) {
    return word.size() < 2 || std::find_if(word.begin(), std::prev(word.end()), [](char c) {
        return is_vowel(c);
    }) == word.end() - 2;
}

void init_irregular_verbs() {
    if (irregular_verbs.empty()) {
        std::ifstream irregular_verbs_file("irregular_verbs.json");
        if (irregular_verbs_file.is_open()) {
            json irregular_verbs_json = json::parse(irregular_verbs_file);
            irregular_verbs.reserve(irregular_verbs_json.size());
            for (const auto& verb : irregular_verbs_json.items()) {
                irregular_verbs[verb.key()] = std::make_pair<std::string, std::string>(verb.value()["past"], verb.value()["past_participle"]);
            }
        }
    }
}

std::string Noun::english_equivalent(const std::string& english_base) const {
    static constexpr const char* prefixes[7] = {
        nullptr,
        "of ",
        "to/for ",
        nullptr,
        nullptr,
        "O ",
        "at ",
    };

    std::string ret;
    if (prefixes[casus]) {
        ret = prefixes[casus] + english_base; // Add prefix
    } else {
        ret = english_base;
    }

    // Add suffix
    if (plural) {
        switch (ret.back()) {
        case 'y':
            if (is_short(english_base)) {
                break;
            }
            ret.back() = 'i';
            goto add_e;

        case 'x':
            if (english_base.size() >= 2 &&
                (english_base[english_base.size() - 2] == 't' ||
                    english_base[english_base.size() - 2] == 'p')) {
                break;
            }
        case 'a':
        case 'i':
        case 'o':
        case 'h':
        case 's':
        add_e:
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

    if (conjugation == 5 && english_base == "be") {
        if (be[mood][tense][person][plural]) {
            return be[mood][tense][person][plural];
        }
    }

    std::string ret;
    if (prefixes[voice][mood][tense][person][plural]) {
        ret = prefixes[voice][mood][tense][person][plural]; // Add prefix
    }

    // Add suffix
    init_irregular_verbs();
    switch (voice) {
    case VOICE_ACTIVE:
        switch (mood) {
        case MOOD_INDICATIVE:
            switch (tense) {
            case TENSE_PRESENT:
                ret += english_base;
                if (person == 2 && !plural) {
                    switch (ret.back()) {
                    case 'y':
                        if (is_short(english_base)) {
                            break;
                        }
                        ret.back() = 'i';
                        goto add_e;

                    case 'x':
                        if (english_base.size() >= 2 &&
                            (english_base[english_base.size() - 2] == 't' ||
                                english_base[english_base.size() - 2] == 'p')) {
                            break;
                        }
                    case 'a':
                    case 'i':
                    case 'o':
                    case 'h':
                    case 's':
                    add_e:
                        ret.push_back('e');
                        break;
                    }
                    ret.push_back('s');
                }
                break;

            case TENSE_IMPERFECT:
                ret += english_base;
                if (ret.back() == 'e') {
                    ret.back() = 'i';
                    ret += "ng";
                } else {
                    if (is_short(english_base) &&
                        is_consonant(english_base.back(), false) &&
                        tolower(english_base.back()) != 'w') {
                        ret.push_back(ret.back());
                    }
                    ret += "ing";
                }
                break;

            case TENSE_PERFECT:
            case TENSE_PLUPERFECT:
            case TENSE_FUTURE_PERFECT: {
                decltype(irregular_verbs)::const_iterator irregular_verb_it;
                if ((irregular_verb_it = irregular_verbs.find(english_base)) != irregular_verbs.end()) {
                    ret += tense == TENSE_PERFECT ? irregular_verb_it->second.first : irregular_verb_it->second.second;
                } else {
                    ret += english_base;
                    if (ret.back() == 'e') {
                        ret.push_back('d');
                        break;
                    } else if (ret.back() == 'y') {
                        ret.back() = 'i';
                    } else if (is_short(english_base) &&
                               is_consonant(english_base.back(), false) &&
                               tolower(english_base.back()) != 'w') {
                        ret.push_back(ret.back());
                    }
                    ret += "ed";
                }
                break;
            }

            default:
                ret += english_base;
                break;
            }
            break;

        case MOOD_SUBJUNCTIVE:
            switch (tense) {
            case TENSE_PERFECT:
            case TENSE_PLUPERFECT: {
                decltype(irregular_verbs)::const_iterator irregular_verb_it;
                if ((irregular_verb_it = irregular_verbs.find(english_base)) != irregular_verbs.end()) {
                    ret += irregular_verb_it->second.second;
                } else {
                    ret += english_base;
                    if (ret.back() == 'e') {
                        ret.push_back('d');
                        break;
                    } else if (ret.back() == 'y') {
                        ret.back() = 'i';
                    } else if (is_short(english_base) &&
                               is_consonant(english_base.back(), false) &&
                               tolower(english_base.back()) != 'w') {
                        ret.push_back(ret.back());
                    }
                    ret += "ed";
                }
                break;
            }

            default:
                ret += english_base;
                break;
            }
            break;

        case MOOD_INFINITIVE:
            if (tense == TENSE_PERFECT) {
                decltype(irregular_verbs)::const_iterator irregular_verb_it;
                if ((irregular_verb_it = irregular_verbs.find(english_base)) != irregular_verbs.end()) {
                    ret += irregular_verb_it->second.second;
                } else {
                    ret += english_base;
                    if (ret.back() == 'e') {
                        ret.push_back('d');
                        break;
                    } else if (ret.back() == 'y') {
                        ret.back() = 'i';
                    } else if (is_short(english_base) &&
                               is_consonant(english_base.back(), false) &&
                               tolower(english_base.back()) != 'w') {
                        ret.push_back(ret.back());
                    }
                    ret += "ed";
                }
                break;
            } else {
                ret += english_base;
            }
            break;

        default:
            ret += english_base;
            break;
        }
        break;

    case VOICE_PASSIVE: {
        decltype(irregular_verbs)::const_iterator irregular_verb_it;
        if ((irregular_verb_it = irregular_verbs.find(english_base)) != irregular_verbs.end()) {
            ret += irregular_verb_it->second.second;
        } else {
            ret += english_base;
            if (ret.back() == 'e') {
                ret.push_back('d');
                break;
            } else if (ret.back() == 'y') {
                ret.back() = 'i';
            } else if (is_short(english_base) &&
                       is_consonant(english_base.back(), false) &&
                       tolower(english_base.back()) != 'w') {
                ret.push_back(ret.back());
            }
            ret += "ed";
        }
        break;
    }

    default:
        throw std::logic_error("Invalid gender");
    }

    return ret;
}

std::string Participle::english_equivalent(const std::string& english_base) const {
    static constexpr const char* prefixes[2][6] = {
        // Active voice
        {
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            "about to ", // Future tense
        },
        // Passive voice
        {
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            "to be ", // Future tense
        },
    };

    std::string ret;
    if (prefixes[voice][tense]) {
        ret = prefixes[voice][tense]; // Add prefix
    }

    // Add suffix
    init_irregular_verbs();
    switch (voice) {
    case VOICE_ACTIVE:
        ret += english_base;
        if (tense == TENSE_PRESENT) {
            if (ret.back() == 'e') {
                ret.back() = 'i';
                ret += "ng";
            } else {
                if (is_short(english_base) &&
                    is_consonant(english_base.back(), false) &&
                    tolower(english_base.back()) != 'w') {
                    ret.push_back(ret.back());
                }
                ret += "ing";
            }
        }
        break;

    case VOICE_PASSIVE: {
        decltype(irregular_verbs)::const_iterator irregular_verb_it;
        if ((irregular_verb_it = irregular_verbs.find(english_base)) != irregular_verbs.end()) {
            ret += irregular_verb_it->second.second;
        } else {
            ret += english_base;
            if (ret.back() == 'e') {
                ret.push_back('d');
                break;
            } else if (ret.back() == 'y') {
                ret.back() = 'i';
            } else if (is_short(english_base) &&
                       is_consonant(english_base.back(), false) &&
                       tolower(english_base.back()) != 'w') {
                ret.push_back(ret.back());
            }
            ret += "ed";
        }
        break;
    }

    default:
        throw std::logic_error("Invalid voice");
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
        return english_base;
    }
}

std::string Adverb::english_equivalent(const std::string& english_base) const {
    if (degree) {
        std::string ret = english_base;
        if (pw::string::ends_with(english_base, "ly")) {
            ret.erase(ret.size() - 2);
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
    }

    return english_base;
}

std::string Pronoun::english_equivalent(const std::string& english_base) const {
    static constexpr const char* he[5][4][2] = {
        // Nominative case
        {
            {"he", "they"},
            {"she", "they"},
            {"it", "they"},
            {"they", "they"},
        },
        // Genitive case
        {
            {"of him", "of them"},
            {"of her", "of them"},
            {"of it", "of them"},
            {"of them", "of them"},
        },
        // Dative case
        {
            {"to/for him", "to/for them"},
            {"to/for her", "to/for them"},
            {"to/for it", "to/for them"},
            {"to/for them", "to/for them"},
        },
        // Accusative case
        {
            {"him", "them"},
            {"her", "them"},
            {"it", "them"},
            {"them", "them"},
        },
        // Ablative case
        {
            {"him", "them"},
            {"her", "them"},
            {"it", "them"},
            {"them", "them"},
        },
    };
    static constexpr const char* himself[5][4][2] = {
        // Nominative case
        {
            {"himself", "themselves"},
            {"herself", "themselves"},
            {"itself", "themselves"},
            {"themself", "themselves"},
        },
        // Genitive case
        {
            {"of himself", "of themselves"},
            {"of herself", "of themselves"},
            {"of itself", "of themselves"},
            {"of themself", "of themselves"},
        },
        // Dative case
        {
            {"to/for himself", "to/for themselves"},
            {"to/for herself", "to/for themselves"},
            {"to/for itself", "to/for themselves"},
            {"to/for themself", "to/for themselves"},
        },
        // Accusative case
        {
            {"himself", "themselves"},
            {"herself", "themselves"},
            {"itself", "themselves"},
            {"themself", "themselves"},
        },
        // Ablative case
        {
            {"himself", "themselves"},
            {"herself", "themselves"},
            {"itself", "themselves"},
            {"themself", "themselves"},
        },
    };
    static constexpr const char* prefixes[7] = {
        nullptr,
        "of ",
        "to/for ",
        nullptr,
        nullptr,
        "O ",
        "at ",
    };

    switch (declension) {
    case 4:
        if (english_base == "he" && he[casus][gender][plural]) {
            return he[casus][gender][plural];
        }
        break;

    case 6:
        if (english_base == "himself" && himself[casus][gender][plural]) {
            return himself[casus][gender][plural];
        }
        break;
    }

    if (prefixes[casus]) {
        return prefixes[casus] + english_base; // Add prefix
    } else {
        return english_base;
    }
}

std::string Numeral::english_equivalent(const std::string& english_base) const {
    std::string ret = english_base;
    if (type == NUMERAL_TYPE_ORDINAL) {
        switch (ret[ret.size() - 3]) { // The english base should be at least 3 characters
        case '1':
            memcpy(&ret[ret.size() - 2], "st", 2);
            break;

        case '2':
            memcpy(&ret[ret.size() - 2], "nd", 2);
            break;

        case '3':
            memcpy(&ret[ret.size() - 2], "rd", 2);
            break;
        }
    }
    return ret;
}
