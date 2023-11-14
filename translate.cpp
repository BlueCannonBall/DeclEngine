#include "Polyweb/string.hpp"
#include "words.hpp"
#include <stdexcept>

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
            ret.back() = 'i';
        case 'a':
        case 'i':
        case 'o':
        case 'u':
        case 'h':
        case 's':
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
                    break;
                } else if (ret.back() == 'y') {
                    ret.back() = 'i';
                }
                ret += "ed";
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
                    break;
                } else if (ret.back() == 'y') {
                    ret.back() = 'i';
                }
                ret += "ed";
                break;

            default:
                break;
            }
            break;

        case MOOD_INFINITIVE:
            if (tense == TENSE_PERFECT) {
                if (ret.back() == 'e') {
                    ret.push_back('d');
                    break;
                } else if (ret.back() == 'y') {
                    ret.back() = 'i';
                }
                ret += "ed";
            }
            break;

        default:
            break;
        }
        break;

    case VOICE_PASSIVE:
        if (ret.back() == 'e') {
            ret.push_back('d');
            break;
        } else if (ret.back() == 'y') {
            ret.back() = 'i';
        }
        ret += "ed";
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
        ret = prefixes[casus] + english_base; // Add prefix
    } else {
        ret = english_base;
    }

    // Add suffix
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
