#include "json.hpp"
#include "words.hpp"
#include <stdexcept>

using nlohmann::json;

json WordForm::to_json() const {
    json ret;

    switch (part_of_speech) {
    case PART_OF_SPEECH_NOUN: ret["part_of_speech"] = "noun"; break;
    case PART_OF_SPEECH_VERB: ret["part_of_speech"] = "verb"; break;
    case PART_OF_SPEECH_PARTICIPLE: ret["part_of_speech"] = "participle"; break;
    case PART_OF_SPEECH_SUPINE: ret["part_of_speech"] = "supine"; break;
    case PART_OF_SPEECH_ADJECTIVE: ret["part_of_speech"] = "adjective"; break;
    case PART_OF_SPEECH_ADVERB: ret["part_of_speech"] = "adverb"; break;
    case PART_OF_SPEECH_PRONOUN: ret["part_of_speech"] = "pronoun"; break;
    case PART_OF_SPEECH_CONJUNCTION: ret["part_of_speech"] = "conjunction"; break;
    case PART_OF_SPEECH_PREPOSITION: ret["part_of_speech"] = "preposition"; break;
    case PART_OF_SPEECH_INTERJECTION: ret["part_of_speech"] = "interjection"; break;
    }

    return ret;
}

json Noun::to_json() const {
    json ret = WordForm::to_json();

    switch (casus) {
    case CASUS_NOMINATIVE: ret["casus"] = "nominative"; break;
    case CASUS_GENITIVE: ret["casus"] = "genitive"; break;
    case CASUS_DATIVE: ret["casus"] = "dative"; break;
    case CASUS_ACCUSATIVE: ret["casus"] = "accusative"; break;
    case CASUS_ABLATIVE: ret["casus"] = "ablative"; break;
    case CASUS_LOCATIVE: ret["casus"] = "locative"; break;
    case CASUS_VOCATIVE: ret["casus"] = "vocative"; break;
    default: throw std::logic_error("Invalid case");
    }

    ret["plural"] = plural;

    switch (gender) {
    case GENDER_MASCULINE: ret["gender"] = "masculine"; break;
    case GENDER_FEMININE: ret["gender"] = "feminine"; break;
    case GENDER_NEUTER: ret["gender"] = "neuter"; break;
    case GENDER_COMMON: ret["gender"] = "common"; break;
    default: throw std::logic_error("Invalid gender");
    }

    return ret;
}

json Verb::to_json() const {
    json ret = WordForm::to_json();

    switch (tense) {
    case TENSE_PRESENT: ret["tense"] = "present"; break;
    case TENSE_IMPERFECT: ret["tense"] = "imperfect"; break;
    case TENSE_PERFECT: ret["tense"] = "perfect"; break;
    case TENSE_PLUPERFECT: ret["tense"] = "pluperfect"; break;
    case TENSE_FUTURE: ret["tense"] = "future"; break;
    case TENSE_FUTURE_PERFECT: ret["tense"] = "future_perfect"; break;
    default: throw std::logic_error("Invalid tense");
    }

    switch (voice) {
    case VOICE_ACTIVE: ret["voice"] = "active"; break;
    case VOICE_PASSIVE: ret["voice"] = "passive"; break;
    default: throw std::logic_error("Invalid voice");
    }

    switch (mood) {
    case MOOD_INDICATIVE: ret["mood"] = "indicative"; break;
    case MOOD_SUBJUNCTIVE: ret["mood"] = "subjunctive"; break;
    case MOOD_IMPERATIVE: ret["mood"] = "imperative"; break;
    case MOOD_INFINITIVE: ret["mood"] = "infinitive"; break;
    default: throw std::logic_error("Invalid mood");
    }

    ret["plural"] = plural;

    return ret;
}

json Participle::to_json() const {
    json ret = WordForm::to_json();

    switch (casus) {
    case CASUS_NOMINATIVE: ret["casus"] = "nominative"; break;
    case CASUS_GENITIVE: ret["casus"] = "genitive"; break;
    case CASUS_DATIVE: ret["casus"] = "dative"; break;
    case CASUS_ACCUSATIVE: ret["casus"] = "accusative"; break;
    case CASUS_ABLATIVE: ret["casus"] = "ablative"; break;
    case CASUS_LOCATIVE: ret["casus"] = "locative"; break;
    case CASUS_VOCATIVE: ret["casus"] = "vocative"; break;
    default: throw std::logic_error("Invalid case");
    }

    ret["plural"] = plural;

    switch (gender) {
    case GENDER_MASCULINE: ret["gender"] = "masculine"; break;
    case GENDER_FEMININE: ret["gender"] = "feminine"; break;
    case GENDER_NEUTER: ret["gender"] = "neuter"; break;
    case GENDER_COMMON: ret["gender"] = "common"; break;
    default: throw std::logic_error("Invalid gender");
    }

    switch (tense) {
    case TENSE_PRESENT: ret["tense"] = "present"; break;
    case TENSE_IMPERFECT: ret["tense"] = "imperfect"; break;
    case TENSE_PERFECT: ret["tense"] = "perfect"; break;
    case TENSE_PLUPERFECT: ret["tense"] = "pluperfect"; break;
    case TENSE_FUTURE: ret["tense"] = "future"; break;
    case TENSE_FUTURE_PERFECT: ret["tense"] = "future_perfect"; break;
    default: throw std::logic_error("Invalid tense");
    }

    switch (voice) {
    case VOICE_ACTIVE: ret["voice"] = "active"; break;
    case VOICE_PASSIVE: ret["voice"] = "passive"; break;
    default: throw std::logic_error("Invalid voice");
    }

    return ret;
}

json Supine::to_json() const {
    json ret = WordForm::to_json();

    switch (casus) {
    case CASUS_NOMINATIVE: ret["casus"] = "nominative"; break;
    case CASUS_GENITIVE: ret["casus"] = "genitive"; break;
    case CASUS_DATIVE: ret["casus"] = "dative"; break;
    case CASUS_ACCUSATIVE: ret["casus"] = "accusative"; break;
    case CASUS_ABLATIVE: ret["casus"] = "ablative"; break;
    case CASUS_LOCATIVE: ret["casus"] = "locative"; break;
    case CASUS_VOCATIVE: ret["casus"] = "vocative"; break;
    default: throw std::logic_error("Invalid case");
    }

    ret["plural"] = plural;

    switch (gender) {
    case GENDER_MASCULINE: ret["gender"] = "masculine"; break;
    case GENDER_FEMININE: ret["gender"] = "feminine"; break;
    case GENDER_NEUTER: ret["gender"] = "neuter"; break;
    case GENDER_COMMON: ret["gender"] = "common"; break;
    default: throw std::logic_error("Invalid gender");
    }

    return ret;
}

json Adjective::to_json() const {
    json ret = Noun::to_json();

    switch (degree) {
    case DEGREE_POSITIVE: ret["degree"] = "positive"; break;
    case DEGREE_COMPARATIVE: ret["degree"] = "comparative"; break;
    case DEGREE_SUPERLATIVE: ret["degree"] = "superlative"; break;
    default: throw std::logic_error("Invalid degree");
    }

    return ret;
}

json Adverb::to_json() const {
    json ret = WordForm::to_json();

    switch (degree) {
    case DEGREE_POSITIVE: ret["degree"] = "positive"; break;
    case DEGREE_COMPARATIVE: ret["degree"] = "comparative"; break;
    case DEGREE_SUPERLATIVE: ret["degree"] = "superlative"; break;
    default: throw std::logic_error("Invalid gender");
    }

    return ret;
}

json Preposition::to_json() const {
    json ret = WordForm::to_json();

    switch (casus) {
    case CASUS_NOMINATIVE: ret["casus"] = "nominative"; break;
    case CASUS_GENITIVE: ret["casus"] = "genitive"; break;
    case CASUS_DATIVE: ret["casus"] = "dative"; break;
    case CASUS_ACCUSATIVE: ret["casus"] = "accusative"; break;
    case CASUS_ABLATIVE: ret["casus"] = "ablative"; break;
    case CASUS_LOCATIVE: ret["casus"] = "locative"; break;
    case CASUS_VOCATIVE: ret["casus"] = "vocative"; break;
    default: throw std::logic_error("Invalid case");
    }

    return ret;
}
