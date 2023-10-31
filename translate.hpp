#pragma once

#include <memory>
#include <string>
#include <vector>

enum PartOfSpeech {
    PART_OF_SPEECH_NOUN,
    PART_OF_SPEECH_VERB,
    PART_OF_SPEECH_ADJECTIVE,
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
    MOOD_INFINITIVE,
    MOOD_IMPERATIVE,
};

typedef unsigned short Declension;
typedef unsigned short Conjugation;
typedef unsigned short Number;

class WordVariant {
public:
    PartOfSpeech part_of_speech;

    WordVariant(PartOfSpeech part_of_speech):
        part_of_speech(part_of_speech) {}

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
    Number number;
    bool plural;

    Verb(Conjugation conjugation, Tense tense, Voice voice, Mood mood, Number number, bool plural):
        WordVariant(PART_OF_SPEECH_VERB),
        conjugation(conjugation),
        tense(tense),
        voice(voice),
        mood(mood),
        number(number),
        plural(plural) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

class Adjective : public WordVariant {
public:
    Declension declension;
    Casus casus;
    bool plural;
    Gender gender;

    Adjective(Declension declension, Casus casus, bool plural, Gender gender):
        WordVariant(PART_OF_SPEECH_ADJECTIVE),
        declension(declension),
        casus(casus),
        plural(plural),
        gender(gender) {}

    std::string english_equivalent(const std::string& english_base) const override;
};

struct WordInfo {
    std::string split_word;
    std::vector<std::unique_ptr<WordVariant>> variants;
    std::string english_base;

    bool is_valid() const {
        return !variants.empty();
    }
};

std::string remove_accents(const std::string& str);
WordInfo query_whitakers_words(const std::string& word);
