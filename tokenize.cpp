#include "words.hpp"
#include <stdexcept>

std::string WordForm::tokenize() const {
    std::string ret;

    switch (part_of_speech) {
    case PART_OF_SPEECH_NOUN: ret = "<F:N>"; break;
    case PART_OF_SPEECH_VERB: ret = "<F:V>"; break;
    case PART_OF_SPEECH_PARTICIPLE: ret = "<F:PAR>"; break;
    case PART_OF_SPEECH_SUPINE: ret = "<F:S>"; break;
    case PART_OF_SPEECH_ADJECTIVE: ret = "<F:ADJ>"; break;
    case PART_OF_SPEECH_ADVERB: ret = "<F:ADV>"; break;
    case PART_OF_SPEECH_PRONOUN: ret = "<F:PRON>"; break;
    case PART_OF_SPEECH_CONJUNCTION: ret = "<F:C>"; break;
    case PART_OF_SPEECH_PREPOSITION: ret = "<F:PREP>"; break;
    case PART_OF_SPEECH_INTERJECTION: ret = "<F:I>"; break;
    case PART_OF_SPEECH_NUMERAL: ret = "<F:NUM>"; break;
    }

    return ret;
}

std::string Noun::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (casus) {
    case CASUS_NOMINATIVE: ret += "<C:N>"; break;
    case CASUS_GENITIVE: ret += "<C:G>"; break;
    case CASUS_DATIVE: ret += "<C:D>"; break;
    case CASUS_ACCUSATIVE: ret += "<C:ACC>"; break;
    case CASUS_ABLATIVE: ret += "<C:ABL>"; break;
    case CASUS_VOCATIVE: ret += "<C:V>"; break;
    case CASUS_LOCATIVE: ret += "<C:L>"; break;
    default: throw std::logic_error("Invalid case");
    }

    if (plural) {
        ret += "<P:T>";
    } else {
        ret += "<P:F>";
    }

    switch (gender) {
    case GENDER_MASCULINE: ret += "<G:M>"; break;
    case GENDER_FEMININE: ret += "<G:F>"; break;
    case GENDER_NEUTER: ret += "<G:N>"; break;
    case GENDER_COMMON: ret += "<G:C>"; break;
    default: throw std::logic_error("Invalid gender");
    }

    return ret;
}

std::string Verb::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (tense) {
    case TENSE_PRESENT: ret += "<T:PRES>"; break;
    case TENSE_IMPERFECT: ret += "<T:I>"; break;
    case TENSE_PERFECT: ret += "<T:PERF>"; break;
    case TENSE_PLUPERFECT: ret += "<T:PLUP>"; break;
    case TENSE_FUTURE: ret += "<T:F>"; break;
    case TENSE_FUTURE_PERFECT: ret += "<T:FPERF>"; break;
    default: throw std::logic_error("Invalid tense");
    }

    switch (voice) {
    case VOICE_ACTIVE: ret += "<V:A>"; break;
    case VOICE_PASSIVE: ret += "<V:P>"; break;
    default: throw std::logic_error("Invalid voice");
    }

    switch (mood) {
    case MOOD_INDICATIVE: ret += "<M:IND>"; break;
    case MOOD_SUBJUNCTIVE: ret += "<M:S>"; break;
    case MOOD_IMPERATIVE: ret += "<M:IMP>"; break;
    case MOOD_INFINITIVE: ret += "<M:INF>"; break;
    default: throw std::logic_error("Invalid mood");
    }

    ret += "<PPL:" + std::to_string(person + 1) + '>';

    if (plural) {
        ret += "<P:T>";
    } else {
        ret += "<P:F>";
    }

    return ret;
}

std::string Participle::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (casus) {
    case CASUS_NOMINATIVE: ret += "<C:N>"; break;
    case CASUS_GENITIVE: ret += "<C:G>"; break;
    case CASUS_DATIVE: ret += "<C:D>"; break;
    case CASUS_ACCUSATIVE: ret += "<C:ACC>"; break;
    case CASUS_ABLATIVE: ret += "<C:ABL>"; break;
    case CASUS_VOCATIVE: ret += "<C:V>"; break;
    case CASUS_LOCATIVE: ret += "<C:L>"; break;
    default: throw std::logic_error("Invalid case");
    }

    if (plural) {
        ret += "<P:T>";
    } else {
        ret += "<P:F>";
    }

    switch (gender) {
    case GENDER_MASCULINE: ret += "<G:M>"; break;
    case GENDER_FEMININE: ret += "<G:F>"; break;
    case GENDER_NEUTER: ret += "<G:N>"; break;
    case GENDER_COMMON: ret += "<G:C>"; break;
    default: throw std::logic_error("Invalid gender");
    }

    switch (tense) {
    case TENSE_PRESENT: ret += "<T:PRES>"; break;
    case TENSE_IMPERFECT: ret += "<T:I>"; break;
    case TENSE_PERFECT: ret += "<T:PERF>"; break;
    case TENSE_PLUPERFECT: ret += "<T:PLUP>"; break;
    case TENSE_FUTURE: ret += "<T:F>"; break;
    case TENSE_FUTURE_PERFECT: ret += "<T:FPERF>"; break;
    default: throw std::logic_error("Invalid tense");
    }

    switch (voice) {
    case VOICE_ACTIVE: ret += "<V:A>"; break;
    case VOICE_PASSIVE: ret += "<V:P>"; break;
    default: throw std::logic_error("Invalid voice");
    }

    return ret;
}

std::string Supine::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (casus) {
    case CASUS_NOMINATIVE: ret += "<C:N>"; break;
    case CASUS_GENITIVE: ret += "<C:G>"; break;
    case CASUS_DATIVE: ret += "<C:D>"; break;
    case CASUS_ACCUSATIVE: ret += "<C:ACC>"; break;
    case CASUS_ABLATIVE: ret += "<C:ABL>"; break;
    case CASUS_VOCATIVE: ret += "<C:V>"; break;
    case CASUS_LOCATIVE: ret += "<C:L>"; break;
    default: throw std::logic_error("Invalid case");
    }

    if (plural) {
        ret += "<P:T>";
    } else {
        ret += "<P:F>";
    }

    switch (gender) {
    case GENDER_MASCULINE: ret += "<G:M>"; break;
    case GENDER_FEMININE: ret += "<G:F>"; break;
    case GENDER_NEUTER: ret += "<G:N>"; break;
    case GENDER_COMMON: ret += "<G:C>"; break;
    default: throw std::logic_error("Invalid gender");
    }

    return ret;
}

std::string Adjective::tokenize() const {
    std::string ret = Noun::tokenize();

    switch (degree) {
    case DEGREE_POSITIVE: ret += "<D:P>"; break;
    case DEGREE_COMPARATIVE: ret += "<D:C>"; break;
    case DEGREE_SUPERLATIVE: ret += "<D:S>"; break;
    default: throw std::logic_error("Invalid degree");
    }

    return ret;
}

std::string Adverb::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (degree) {
    case DEGREE_POSITIVE: ret += "<D:P>"; break;
    case DEGREE_COMPARATIVE: ret += "<D:C>"; break;
    case DEGREE_SUPERLATIVE: ret += "<D:S>"; break;
    default: throw std::logic_error("Invalid degree");
    }

    return ret;
}

std::string Preposition::tokenize() const {
    std::string ret = WordForm::tokenize();

    switch (casus) {
    case CASUS_NOMINATIVE: ret += "<C:N>"; break;
    case CASUS_GENITIVE: ret += "<C:G>"; break;
    case CASUS_DATIVE: ret += "<C:D>"; break;
    case CASUS_ACCUSATIVE: ret += "<C:ACC>"; break;
    case CASUS_ABLATIVE: ret += "<C:ABL>"; break;
    case CASUS_VOCATIVE: ret += "<C:V>"; break;
    case CASUS_LOCATIVE: ret += "<C:L>"; break;
    default: throw std::logic_error("Invalid case");
    }

    return ret;
}

std::string Numeral::tokenize() const {
    std::string ret = Noun::tokenize();

    switch (type) {
    case NUMERAL_TYPE_CARDINAL: ret += "<N:C>"; break;
    case NUMERAL_TYPE_ORDINAL: ret += "<N:O>"; break;
    case NUMERAL_TYPE_DISTRIBUTIVE: ret += "<N:D>"; break;
    case NUMERAL_TYPE_ADVERB: ret += "<N:A>"; break;
    default: throw std::logic_error("Invalid numeral type");
    }

    return ret;
}
