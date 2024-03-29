#pragma once

#include "json_fwd.hpp"
#include <string>

enum PartOfSpeech {
    PART_OF_SPEECH_NOUN,
    PART_OF_SPEECH_VERB,
    PART_OF_SPEECH_PARTICIPLE,
    PART_OF_SPEECH_SUPINE,
    PART_OF_SPEECH_ADJECTIVE,
    PART_OF_SPEECH_ADVERB,
    PART_OF_SPEECH_PRONOUN,
    PART_OF_SPEECH_CONJUNCTION,
    PART_OF_SPEECH_PREPOSITION,
    PART_OF_SPEECH_INTERJECTION,
    PART_OF_SPEECH_NUMERAL,
};

enum Casus {
    CASUS_NOMINATIVE,
    CASUS_GENITIVE,
    CASUS_DATIVE,
    CASUS_ACCUSATIVE,
    CASUS_ABLATIVE,
    CASUS_VOCATIVE,
    CASUS_LOCATIVE,
    CASUS_NONE,
};

enum Gender {
    GENDER_MASCULINE,
    GENDER_FEMININE,
    GENDER_NEUTER,
    GENDER_COMMON,
    GENDER_NONE,
};

inline bool operator==(Gender a, Gender b) {
    return (int) a == b || (int) a == GENDER_COMMON || (int) b == GENDER_COMMON;
}

inline bool operator!=(Gender a, Gender b) {
    return (int) a != GENDER_COMMON && (int) b != GENDER_COMMON && (int) a != b;
}

enum Degree {
    DEGREE_POSITIVE,
    DEGREE_COMPARATIVE,
    DEGREE_SUPERLATIVE,
    DEGREE_NONE,
};

enum Tense {
    TENSE_PRESENT,
    TENSE_IMPERFECT,
    TENSE_PERFECT,
    TENSE_PLUPERFECT,
    TENSE_FUTURE,
    TENSE_FUTURE_PERFECT,
    TENSE_NONE,
};

enum Voice {
    VOICE_ACTIVE,
    VOICE_PASSIVE,
    VOICE_NONE,
};

enum Mood {
    MOOD_INDICATIVE,
    MOOD_SUBJUNCTIVE,
    MOOD_IMPERATIVE,
    MOOD_INFINITIVE,
    MOOD_NONE,
};

enum NumeralType {
    NUMERAL_TYPE_CARDINAL,
    NUMERAL_TYPE_ORDINAL,
    NUMERAL_TYPE_DISTRIBUTIVE,
    NUMERAL_TYPE_ADVERB,
    NUMERAL_TYPE_NONE,
};

typedef unsigned short Declension;
typedef unsigned short Conjugation;
typedef unsigned short Person;

class WordForm {
public:
    PartOfSpeech part_of_speech;

    WordForm(PartOfSpeech part_of_speech):
        part_of_speech(part_of_speech) {}

    virtual ~WordForm() = default;
    virtual std::string english_equivalent(const std::string& english_base) const = 0;
    virtual std::string tokenize() const;
    virtual nlohmann::json to_json() const;

    virtual Casus get_casus() const { return CASUS_NONE; }
    virtual Gender get_gender() const { return GENDER_NONE; }
    virtual Degree get_degree() const { return DEGREE_NONE; }
    virtual Tense get_tense() const { return TENSE_NONE; }
    virtual Voice get_voice() const { return VOICE_NONE; }
    virtual Mood get_mood() const { return MOOD_NONE; }
    virtual NumeralType get_numeral_type() const { return NUMERAL_TYPE_NONE; }
    virtual Declension get_declension() const { return 0; }
    virtual Conjugation get_conjugation() const { return 0; }
    virtual Person get_person() const { return 0; }
    virtual bool is_plural() const { return false; }

    virtual bool is_noun_like() const { return false; }
};

class Noun : public WordForm {
public:
    Declension declension;
    Casus casus;
    bool plural;
    Gender gender;

    Noun(PartOfSpeech part_of_speech = PART_OF_SPEECH_NOUN):
        WordForm(part_of_speech) {}
    Noun(Declension declension, Casus casus, bool plural, Gender gender):
        WordForm(PART_OF_SPEECH_NOUN),
        declension(declension),
        casus(casus),
        plural(plural),
        gender(gender) {}

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Casus get_casus() const override { return casus; }
    Gender get_gender() const override { return gender; }
    Declension get_declension() const override { return declension; }
    bool is_plural() const override { return plural; }

    bool is_noun_like() const override { return true; }
};

class Verb : public WordForm {
public:
    Conjugation conjugation;
    Tense tense;
    Voice voice;
    Mood mood;
    Person person;
    bool plural;

    Verb(PartOfSpeech part_of_speech = PART_OF_SPEECH_VERB):
        WordForm(part_of_speech) {}
    Verb(Conjugation conjugation, Tense tense, Voice voice, Mood mood, Person person, bool plural):
        WordForm(PART_OF_SPEECH_VERB),
        conjugation(conjugation),
        tense(tense),
        voice(voice),
        mood(mood),
        person(person),
        plural(plural) {}

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Tense get_tense() const override { return tense; }
    Voice get_voice() const override { return voice; }
    Mood get_mood() const override { return mood; }
    Conjugation get_conjugation() const override { return conjugation; }
    Person get_person() const override { return person; }
    bool is_plural() const override { return plural; }
};

class Participle : public WordForm {
public:
    Conjugation conjugation;
    Casus casus;
    bool plural;
    Gender gender;
    Tense tense;
    Voice voice;

    Participle(PartOfSpeech part_of_speech = PART_OF_SPEECH_PARTICIPLE):
        WordForm(part_of_speech) {}
    Participle(Conjugation conjugation, Casus casus, bool plural, Gender gender, Tense tense, Voice voice):
        WordForm(PART_OF_SPEECH_PARTICIPLE),
        conjugation(conjugation),
        casus(casus),
        plural(plural),
        gender(gender),
        tense(tense),
        voice(voice) {}

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Casus get_casus() const override { return casus; }
    Gender get_gender() const override { return gender; }
    Tense get_tense() const override { return tense; }
    Voice get_voice() const override { return voice; }
    Conjugation get_conjugation() const override { return conjugation; }
    bool is_plural() const override { return plural; }

    bool is_noun_like() const override { return true; }
};

class Supine : public WordForm {
public:
    Conjugation conjugation;
    Casus casus;
    bool plural;
    Gender gender;

    Supine(PartOfSpeech part_of_speech = PART_OF_SPEECH_SUPINE):
        WordForm(part_of_speech) {}
    Supine(Conjugation conjugation, Casus casus, bool plural, Gender gender):
        WordForm(PART_OF_SPEECH_SUPINE),
        conjugation(conjugation),
        casus(casus),
        plural(plural),
        gender(gender) {}

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Casus get_casus() const override { return casus; }
    Gender get_gender() const override { return gender; }
    Conjugation get_conjugation() const override { return conjugation; }
    bool is_plural() const override { return plural; }
};

class Adjective : public Noun {
public:
    Degree degree;

    Adjective(PartOfSpeech part_of_speech = PART_OF_SPEECH_ADJECTIVE):
        Noun(part_of_speech) {}
    Adjective(Declension declension, Casus casus, bool plural, Gender gender, Degree degree):
        Noun(declension, casus, plural, gender),
        degree(degree) {
        part_of_speech = PART_OF_SPEECH_ADJECTIVE;
    }

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Degree get_degree() const override { return degree; }

    bool is_noun_like() const override { return false; }
};

class Adverb : public WordForm {
public:
    Degree degree;

    Adverb(PartOfSpeech part_of_speech = PART_OF_SPEECH_ADVERB):
        WordForm(part_of_speech) {}
    Adverb(Degree degree):
        WordForm(PART_OF_SPEECH_ADVERB),
        degree(degree) {}

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Degree get_degree() const override { return degree; }
};

class Pronoun : public Noun {
public:
    Pronoun(PartOfSpeech part_of_speech = PART_OF_SPEECH_PRONOUN):
        Noun(part_of_speech) {}
    Pronoun(Declension declension, Casus casus, bool plural, Gender gender):
        Noun(declension, casus, plural, gender) {
        part_of_speech = PART_OF_SPEECH_PRONOUN;
    }

    std::string english_equivalent(const std::string& english_base) const override;
};

class Conjunction : public WordForm {
public:
    Conjunction(PartOfSpeech part_of_speech = PART_OF_SPEECH_CONJUNCTION):
        WordForm(part_of_speech) {}

    std::string english_equivalent(const std::string& english_base) const override {
        return english_base;
    }
};

class Preposition : public WordForm {
public:
    Casus casus;

    Preposition(PartOfSpeech part_of_speech = PART_OF_SPEECH_PREPOSITION):
        WordForm(part_of_speech) {}
    Preposition(Casus casus):
        WordForm(PART_OF_SPEECH_PREPOSITION),
        casus(casus) {}

    std::string english_equivalent(const std::string& english_base) const override {
        return english_base;
    }

    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    Casus get_casus() const override { return casus; }
};

class Interjection : public WordForm {
public:
    Interjection(PartOfSpeech part_of_speech = PART_OF_SPEECH_INTERJECTION):
        WordForm(part_of_speech) {}

    std::string english_equivalent(const std::string& english_base) const override {
        return english_base;
    }
};

class Numeral : public Noun {
public:
    NumeralType type;

    Numeral(PartOfSpeech part_of_speech = PART_OF_SPEECH_NUMERAL):
        Noun(part_of_speech) {}
    Numeral(Declension declension, Casus casus, bool plural, Gender gender, NumeralType type):
        Noun(declension, casus, plural, gender),
        type(type) {
        part_of_speech = PART_OF_SPEECH_NUMERAL;
    }

    std::string english_equivalent(const std::string& english_base) const override;
    std::string tokenize() const override;
    nlohmann::json to_json() const override;

    NumeralType get_numeral_type() const override { return type; }

    bool is_noun_like() const override { return false; }
};
