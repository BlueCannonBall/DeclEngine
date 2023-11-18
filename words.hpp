#pragma once

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

enum KeyComponent {
    COMPONENT_NONE,
    COMPONENT_SUBJECT,
    COMPONENT_VERB,
    COMPONENT_OBJECT,
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

    virtual KeyComponent component() const {
        return COMPONENT_NONE;
    }
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

    KeyComponent component() const override {
        switch (casus) {
        case CASUS_NOMINATIVE: return COMPONENT_SUBJECT;
        case CASUS_ACCUSATIVE: return COMPONENT_OBJECT;
        default: return COMPONENT_NONE;
        }
    }
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

    KeyComponent component() const override {
        return COMPONENT_VERB;
    }
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
};

class Adjective : public WordForm {
public:
    Declension declension;
    Casus casus;
    bool plural;
    Gender gender;
    Degree degree;

    Adjective(PartOfSpeech part_of_speech = PART_OF_SPEECH_ADJECTIVE):
        WordForm(part_of_speech) {}
    Adjective(Declension declension, Casus casus, bool plural, Gender gender, Degree degree):
        WordForm(PART_OF_SPEECH_ADJECTIVE),
        declension(declension),
        casus(casus),
        plural(plural),
        gender(gender),
        degree(degree) {}

    std::string english_equivalent(const std::string& english_base) const override;
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

    inline std::string english_equivalent(const std::string& english_base) const override {
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

    inline std::string english_equivalent(const std::string& english_base) const override {
        return english_base;
    }
};

class Interjection : public WordForm {
public:
    Interjection(PartOfSpeech part_of_speech = PART_OF_SPEECH_INTERJECTION):
        WordForm(part_of_speech) {}

    inline std::string english_equivalent(const std::string& english_base) const override {
        return english_base;
    }
};
