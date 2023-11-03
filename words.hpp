#pragma once

#include <string>

enum PartOfSpeech {
    PART_OF_SPEECH_NOUN,
    PART_OF_SPEECH_VERB,
    PART_OF_SPEECH_PARTICIPLE,
    PART_OF_SPEECH_SUPINE,
    PART_OF_SPEECH_ADJECTIVE,
    PART_OF_SPEECH_ADVERB,
};

enum Casus {
    CASUS_NOMINATIVE,
    CASUS_GENITIVE,
    CASUS_DATIVE,
    CASUS_ACCUSATIVE,
    CASUS_ABLATIVE,
    CASUS_VOCATIVE,
    CASUS_LOCATIVE,
};

enum Gender {
    GENDER_MASCULINE,
    GENDER_FEMININE,
    GENDER_NEUTER, // Not applicable to proper nouns
    GENDER_COMMON,
};

enum Degree {
    DEGREE_POSITIVE,
    DEGREE_COMPARATIVE,
    DEGREE_SUPERLATIVE,
};

enum Tense {
    TENSE_PRESENT,
    TENSE_IMPERFECT,
    TENSE_PERFECT,
    TENSE_PLUPERFECT,
    TENSE_FUTURE,
    TENSE_FUTURE_PERFECT,
};

enum Voice {
    VOICE_ACTIVE,
    VOICE_PASSIVE,
};

enum Mood {
    MOOD_INDICATIVE,
    MOOD_SUBJUNCTIVE,
    MOOD_IMPERATIVE,
    MOOD_INFINITIVE,
};

typedef unsigned short Declension;
typedef unsigned short Conjugation;
typedef unsigned short Person;

class WordVariant {
public:
    PartOfSpeech part_of_speech;

    WordVariant(PartOfSpeech part_of_speech):
        part_of_speech(part_of_speech) {}

    virtual ~WordVariant() = default;
    virtual std::string english_equivalent(const std::string& english_base) const = 0;
};

class Noun : public WordVariant {
public:
    Declension declension;
    Casus casus;
    bool plural;
    Gender gender;

    Noun(Declension declension, Casus casus, bool plural, Gender gender):
        WordVariant(PART_OF_SPEECH_NOUN),
        declension(declension),
        casus(casus),
        plural(plural),
        gender(gender) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Verb : public WordVariant {
public:
    Conjugation conjugation;
    Tense tense;
    Voice voice;
    Mood mood;
    Person person;
    bool plural;

    Verb(Conjugation conjugation, Tense tense, Voice voice, Mood mood, Person person, bool plural):
        WordVariant(PART_OF_SPEECH_VERB),
        conjugation(conjugation),
        tense(tense),
        voice(voice),
        mood(mood),
        person(person),
        plural(plural) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Participle : public WordVariant {
public:
    Conjugation conjugation;
    Casus casus;
    bool plural;
    Gender gender;
    Tense tense;
    Voice voice;

    Participle(Conjugation conjugation, Casus casus, bool plural, Gender gender, Tense tense, Voice voice):
        WordVariant(PART_OF_SPEECH_PARTICIPLE),
        conjugation(conjugation),
        casus(casus),
        plural(plural),
        gender(gender),
        tense(tense),
        voice(voice) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Supine : public WordVariant {
public:
    Conjugation conjugation;
    Casus casus;
    bool plural;
    Gender gender;

    Supine(Conjugation conjugation, Casus casus, bool plural, Gender gender):
        WordVariant(PART_OF_SPEECH_SUPINE),
        conjugation(conjugation),
        casus(casus),
        plural(plural),
        gender(gender) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Adjective : public WordVariant {
public:
    Declension declension;
    Casus casus;
    bool plural;
    Gender gender;
    Degree degree;

    Adjective(Declension declension, Casus casus, bool plural, Gender gender, Degree degree):
        WordVariant(PART_OF_SPEECH_ADJECTIVE),
        declension(declension),
        casus(casus),
        plural(plural),
        gender(gender),
        degree(degree) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Adverb : public WordVariant {
public:
    Degree degree;

    Adverb(Degree degree):
        WordVariant(PART_OF_SPEECH_ADVERB),
        degree(degree) {}

    std::string english_equivalent(const std::string& english_base) const override;
};
