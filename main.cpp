#include "Polyweb/polyweb.hpp"
#include "json.hpp"
#include "translate.hpp"
#include <fstream>
#include <iostream>
#include <string>

using nlohmann::json;

int main() {
    std::ofstream settings("whitakers-words/WORD.MOD");
    settings << "TRIM_OUTPUT                       Y\n"
                "HAVE_OUTPUT_FILE                  N\n"
                "WRITE_OUTPUT_TO_FILE              N\n"
                "DO_UNKNOWNS_ONLY                  N\n"
                "WRITE_UNKNOWNS_TO_FILE            N\n"
                "IGNORE_UNKNOWN_NAMES              Y\n"
                "IGNORE_UNKNOWN_CAPS               Y\n"
                "DO_COMPOUNDS                      Y\n"
                "DO_FIXES                          Y\n"
                "DO_TRICKS                         Y\n"
                "DO_DICTIONARY_FORMS               N\n"
                "SHOW_AGE                          N\n"
                "SHOW_FREQUENCY                    N\n"
                "DO_EXAMPLES                       N\n"
                "DO_ONLY_MEANINGS                  N\n"
                "DO_STEMS_FOR_UNKNOWN              N\n";
    settings.close();

    settings.open("whitakers-words/WORD.MDV");
    settings << "HAVE_STATISTICS_FILE              N\n"
                "WRITE_STATISTICS_FILE             N\n"
                "SHOW_DICTIONARY                   N\n"
                "SHOW_DICTIONARY_LINE              N\n"
                "SHOW_DICTIONARY_CODES             N\n"
                "DO_PEARSE_CODES                   N\n"
                "DO_ONLY_INITIAL_WORD              N\n"
                "FOR_WORD_LIST_CHECK               N\n"
                "DO_ONLY_FIXES                     N\n"
                "DO_FIXES_ANYWAY                   N\n"
                "USE_PREFIXES                      Y\n"
                "USE_SUFFIXES                      Y\n"
                "USE_TACKONS                       Y\n"
                "DO_MEDIEVAL_TRICKS                Y\n"
                "DO_SYNCOPE                        Y\n"
                "DO_TWO_WORDS                      N\n"
                "INCLUDE_UNKNOWN_CONTEXT           Y\n"
                "NO_MEANINGS                       N\n"
                "OMIT_ARCHAIC                      Y\n"
                "OMIT_MEDIEVAL                     N\n"
                "OMIT_UNCOMMON                     Y\n"
                "DO_I_FOR_J                        Y\n"
                "DO_U_FOR_V                        N\n"
                "PAUSE_IN_SCREEN_OUTPUT            Y\n"
                "NO_SCREEN_ACTIVITY                N\n"
                "UPDATE_LOCAL_DICTIONARY           N\n"
                "UPDATE_MEANINGS                   N\n"
                "MINIMIZE_OUTPUT                   Y\n"
                "START_FILE_CHARACTER             '@'\n"
                "CHANGE_PARAMETERS_CHARACTER      '#'\n"
                "CHANGE_DEVELOPER_MODES_CHARACTER '!'\n";
    settings.close();

    pn::init();
    pn::UniqueSock<pw::Server> server;

    server->route("/word_info",
        pw::HTTPRoute {
            [](const pw::Connection&, const pw::HTTPRequest& req, void*) {
                pw::QueryParameters::map_type::const_iterator word_it;
                if ((word_it = req.query_parameters->find("word")) == req.query_parameters->end() ||
                    word_it->second.find(' ') != std::string::npos) {
                    return pw::HTTPResponse::make_basic(400);
                }

                WordInfo word_info = query_whitakers_words(word_it->second);
                if (word_info.is_valid()) {
                    json resp = {
                        {"variants", json::array()},
                        {"english_base", word_info.english_base},
                    };

                    for (const auto& variant : word_info.variants) {
                        json json_variant;

                        switch (variant->part_of_speech) {
                        case PART_OF_SPEECH_NOUN: {
                            json_variant["part_of_speech"] = "noun";
                            auto noun = (Noun*) variant.get();

                            switch (noun->casus) {
                            case CASUS_NOMINATIVE: json_variant["case"] = "nominative"; break;
                            case CASUS_GENITIVE: json_variant["case"] = "genitive"; break;
                            case CASUS_DATIVE: json_variant["case"] = "dative"; break;
                            case CASUS_ACCUSATIVE: json_variant["case"] = "accusative"; break;
                            case CASUS_ABLATIVE: json_variant["case"] = "ablative"; break;
                            case CASUS_VOCATIVE: json_variant["case"] = "vocative"; break;
                            case CASUS_LOCATIVE: json_variant["case"] = "locative"; break;
                            }

                            json_variant["plural"] = noun->plural;

                            switch (noun->gender) {
                            case GENDER_MASCULINE: json_variant["gender"] = "masculine"; break;
                            case GENDER_FEMININE: json_variant["gender"] = "feminine"; break;
                            case GENDER_NEUTER: json_variant["gender"] = "neuter"; break;
                            case GENDER_COMMON: json_variant["gender"] = "common"; break;
                            }

                            break;
                        }

                        case PART_OF_SPEECH_VERB: {
                            json_variant["part_of_speech"] = "verb";
                            auto verb = (Verb*) variant.get();

                            json_variant["conjugation"] = verb->conjugation;

                            switch (verb->tense) {
                            case TENSE_PRESENT: json_variant["tense"] = "present"; break;
                            case TENSE_IMPERFECT: json_variant["tense"] = "imperfect"; break;
                            case TENSE_PERFECT: json_variant["tense"] = "perfect"; break;
                            case TENSE_PLUPERFECT: json_variant["tense"] = "pluperfect"; break;
                            case TENSE_FUTURE: json_variant["tense"] = "future"; break;
                            case TENSE_FUTURE_PERFECT: json_variant["tense"] = "future_perfect"; break;
                            }

                            switch (verb->voice) {
                            case VOICE_ACTIVE: json_variant["voice"] = "active"; break;
                            case VOICE_PASSIVE: json_variant["voice"] = "passive"; break;
                            }

                            switch (verb->mood) {
                            case MOOD_INDICATIVE: json_variant["mood"] = "indicative"; break;
                            case MOOD_SUBJUNCTIVE: json_variant["mood"] = "subjunctive"; break;
                            case MOOD_IMPERATIVE: json_variant["mood"] = "imperative"; break;
                            case MOOD_INFINITIVE: json_variant["mood"] = "infinitive"; break;
                            }

                            json_variant["plural"] = verb->plural;

                            break;
                        }

                        case PART_OF_SPEECH_PARTICIPLE: {
                            json_variant["part_of_speech"] = "participle";
                            auto participle = (Participle*) variant.get();

                            json_variant["conjugation"] = participle->conjugation;

                            switch (participle->casus) {
                            case CASUS_NOMINATIVE: json_variant["case"] = "nominative"; break;
                            case CASUS_GENITIVE: json_variant["case"] = "genitive"; break;
                            case CASUS_DATIVE: json_variant["case"] = "dative"; break;
                            case CASUS_ACCUSATIVE: json_variant["case"] = "accusative"; break;
                            case CASUS_ABLATIVE: json_variant["case"] = "ablative"; break;
                            case CASUS_VOCATIVE: json_variant["case"] = "vocative"; break;
                            case CASUS_LOCATIVE: json_variant["case"] = "locative"; break;
                            }

                            json_variant["plural"] = participle->plural;

                            switch (participle->gender) {
                            case GENDER_MASCULINE: json_variant["gender"] = "masculine"; break;
                            case GENDER_FEMININE: json_variant["gender"] = "feminine"; break;
                            case GENDER_NEUTER: json_variant["gender"] = "neuter"; break;
                            case GENDER_COMMON: json_variant["gender"] = "common"; break;
                            }

                            switch (participle->tense) {
                            case TENSE_PRESENT: json_variant["tense"] = "present"; break;
                            case TENSE_IMPERFECT: json_variant["tense"] = "imperfect"; break;
                            case TENSE_PERFECT: json_variant["tense"] = "perfect"; break;
                            case TENSE_PLUPERFECT: json_variant["tense"] = "pluperfect"; break;
                            case TENSE_FUTURE: json_variant["tense"] = "future"; break;
                            case TENSE_FUTURE_PERFECT: json_variant["tense"] = "future_perfect"; break;
                            }

                            switch (participle->voice) {
                            case VOICE_ACTIVE: json_variant["voice"] = "active"; break;
                            case VOICE_PASSIVE: json_variant["voice"] = "passive"; break;
                            }

                            break;
                        }

                        case PART_OF_SPEECH_ADJECTIVE: {
                            json_variant["part_of_speech"] = "adjective";
                            auto adjective = (Adjective*) variant.get();

                            switch (adjective->casus) {
                            case CASUS_NOMINATIVE: json_variant["casus"] = "nominative"; break;
                            case CASUS_GENITIVE: json_variant["casus"] = "genitive"; break;
                            case CASUS_DATIVE: json_variant["casus"] = "dative"; break;
                            case CASUS_ACCUSATIVE: json_variant["casus"] = "accusative"; break;
                            case CASUS_ABLATIVE: json_variant["casus"] = "ablative"; break;
                            case CASUS_VOCATIVE: json_variant["casus"] = "vocative"; break;
                            case CASUS_LOCATIVE: json_variant["casus"] = "locative"; break;
                            }

                            json_variant["plural"] = adjective->plural;

                            switch (adjective->gender) {
                            case GENDER_MASCULINE: json_variant["gender"] = "masculine"; break;
                            case GENDER_FEMININE: json_variant["gender"] = "feminine"; break;
                            case GENDER_NEUTER: json_variant["gender"] = "neuter"; break;
                            case GENDER_COMMON: json_variant["gender"] = "common"; break;
                            }

                            switch (adjective->degree) {
                            case DEGREE_POSITIVE: json_variant["degree"] = "positive"; break;
                            case DEGREE_COMPARATIVE: json_variant["degree"] = "comparative"; break;
                            case DEGREE_SUPERLATIVE: json_variant["degree"] = "superlative"; break;
                            }

                            break;
                        }
                        }

                        json_variant["english_equivalent"] = variant->english_equivalent(word_info.english_base);
                        resp["variants"].push_back(json_variant);
                    }

                    return pw::HTTPResponse(200, resp.dump(4), {{"Content-Type", "application/json"}});
                } else {
                    return pw::HTTPResponse::make_basic(404);
                }
            },
        });

    if (server->bind("0.0.0.0", 8000) == PN_ERROR) {
        std::cerr << "Error: " << pn::universal_strerror() << std::endl;
        return 1;
    }

    std::cout << "Latin Declengine listening on port 8000" << std::endl;
    if (server->listen() == PN_ERROR) {
        std::cerr << "Error: " << pw::universal_strerror() << std::endl;
        return 1;
    }

    pn::quit();
    return 0;
}
