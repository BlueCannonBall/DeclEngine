#include "Polyweb/polyweb.hpp"
#include "dictionary.hpp"
#include "json.hpp"
#include "words.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

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
                "DO_SYNCOPE                        N\n"
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

                std::vector<WordInfo> words;
                if (query_dictionary(word_it->second, words)) {
                    json resp;
                    for (const auto& word_info : words) {
                        json json_word_info = {
                            {"variants", json::array()},
                            {"english_base", word_info.english_base},
                        };

                        for (const auto& variant : word_info.variants) {
                            json json_variant;

                            switch (variant->part_of_speech) {
                            case PART_OF_SPEECH_NOUN:
                            case PART_OF_SPEECH_PRONOUN: {
                                if (variant->part_of_speech == PART_OF_SPEECH_NOUN) {
                                    json_variant["part_of_speech"] = "noun";
                                } else if (variant->part_of_speech == PART_OF_SPEECH_PRONOUN) {
                                    json_variant["part_of_speech"] = "pronoun";
                                } else {
                                    throw std::logic_error("Invalid part of speech");
                                }
                                auto noun = (Noun*) variant.get();

                                switch (noun->casus) {
                                case CASUS_NOMINATIVE: json_variant["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_variant["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_variant["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_variant["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_variant["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_variant["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_variant["casus"] = "locative"; break;
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

                                switch (participle->casus) {
                                case CASUS_NOMINATIVE: json_variant["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_variant["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_variant["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_variant["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_variant["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_variant["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_variant["casus"] = "locative"; break;
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

                            case PART_OF_SPEECH_SUPINE: {
                                json_variant["part_of_speech"] = "supine";
                                auto supine = (Supine*) variant.get();

                                switch (supine->casus) {
                                case CASUS_NOMINATIVE: json_variant["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_variant["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_variant["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_variant["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_variant["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_variant["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_variant["casus"] = "locative"; break;
                                }

                                json_variant["plural"] = supine->plural;

                                switch (supine->gender) {
                                case GENDER_MASCULINE: json_variant["gender"] = "masculine"; break;
                                case GENDER_FEMININE: json_variant["gender"] = "feminine"; break;
                                case GENDER_NEUTER: json_variant["gender"] = "neuter"; break;
                                case GENDER_COMMON: json_variant["gender"] = "common"; break;
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

                            case PART_OF_SPEECH_ADVERB: {
                                json_variant["part_of_speech"] = "adverb";
                                auto adverb = (Adverb*) variant.get();

                                switch (adverb->degree) {
                                case DEGREE_POSITIVE: json_variant["degree"] = "positive"; break;
                                case DEGREE_COMPARATIVE: json_variant["degree"] = "comparative"; break;
                                case DEGREE_SUPERLATIVE: json_variant["degree"] = "superlative"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_CONJUNCTION: {
                                json_variant["part_of_speech"] = "conjunction";
                                break;
                            }

                            case PART_OF_SPEECH_PREPOSITION: {
                                json_variant["part_of_speech"] = "preposition";
                                auto preposition = (Preposition*) variant.get();

                                switch (preposition->casus) {
                                case CASUS_NOMINATIVE: json_variant["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_variant["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_variant["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_variant["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_variant["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_variant["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_variant["casus"] = "locative"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_INTERJECTION: {
                                json_variant["part_of_speech"] = "interjection";
                                break;
                            }
                            }

                            json_variant["english_equivalent"] = variant->english_equivalent(word_info.english_base);
                            json_word_info["variants"].push_back(json_variant);
                        }

                        resp.push_back(json_word_info);
                    }

                    return pw::HTTPResponse(200, resp.dump(4), {{"Content-Type", "application/json"}});
                } else {
                    return pw::HTTPResponse::make_basic(404);
                }
            },
        });

    server->route("/sentence_info",
        pw::HTTPRoute {
            [](const pw::Connection&, const pw::HTTPRequest& req, void*) {
                pw::QueryParameters::map_type::const_iterator input_sentence_it;
                if ((input_sentence_it = req.query_parameters->find("sentence")) == req.query_parameters->end()) {
                    return pw::HTTPResponse::make_basic(400);
                }

                std::vector<std::string> split_input_sentence = pw::string::split_and_trim(input_sentence_it->second, ' ');
                std::vector<WordInfo> input_words;
                for (const auto& string_word : split_input_sentence) {
                    std::vector<WordInfo> possible_words;
                    if (query_dictionary(string_word, possible_words)) {
                        auto word_info_it = std::min_element(possible_words.begin(), possible_words.end(), [](const auto& a, const auto& b) {
                            return a.english_base.size() < b.english_base.size();
                        });
                        input_words.push_back(std::move(*word_info_it));
                    }
                }

                // Eliminate impossible variants
                for (size_t i = 0; i < input_words.size() - 1; ++i) {
                    size_t j = i + 1;
                    for (const auto& variant : input_words[i].variants) {
                        if (auto preposition = dynamic_cast<Preposition*>(variant.get())) {
                            input_words[j].variants.erase(std::remove_if(input_words[j].variants.begin(), input_words[j].variants.end(), [preposition](const auto& variant) {
                                Noun* noun;
                                return (noun = dynamic_cast<Noun*>(variant.get())) && noun->casus != preposition->casus;
                            }),
                                input_words[j].variants.end());
                        }
                    }
                }

                std::vector<std::pair<std::string, std::shared_ptr<WordVariant>>> output_variants;
                {
                    std::vector<std::shared_ptr<Noun>> subjects;
                    std::vector<std::shared_ptr<Verb>> verbs;
                    std::vector<std::shared_ptr<Noun>> objects;
                    for (const auto& word : input_words) {
                        bool found_variant = false;
                        switch (word.variants.front()->part_of_speech) {
                        case PART_OF_SPEECH_NOUN:
                        case PART_OF_SPEECH_PRONOUN:
                            for (const auto& variant : word.variants) {
                                auto noun = (Noun*) variant.get();
                                if (subjects.size() <= objects.size() && noun->casus == CASUS_NOMINATIVE) {
                                    subjects.push_back(std::static_pointer_cast<Noun>(variant));
                                    output_variants.push_back({word.english_base, variant});
                                    found_variant = true;
                                    break;
                                } else if (noun->casus == CASUS_ACCUSATIVE) {
                                    objects.push_back(std::static_pointer_cast<Noun>(variant));
                                    output_variants.push_back({word.english_base, variant});
                                    found_variant = true;
                                    break;
                                }
                            }
                            break;

                        case PART_OF_SPEECH_VERB:
                            for (const auto& variant : word.variants) {
                                auto verb = (Verb*) variant.get();
                                if (subjects.empty() || verb->plural == subjects.back()->plural) {
                                    verbs.push_back(std::static_pointer_cast<Verb>(variant));
                                    output_variants.push_back({word.english_base, variant});
                                    found_variant = true;
                                    break;
                                }
                            }
                            break;

                        default:
                            break;
                        }

                        if (!found_variant) {
                            output_variants.push_back({word.english_base, word.variants.front()});
                        }
                    }
                }

                // Reorder words
                {
                    bool done;
                    do {
                        done = true;
                        decltype(output_variants)::iterator last_key_variant_it = output_variants.end();
                        for (auto it = output_variants.begin(); it != output_variants.end(); ++it) {
                            if (last_key_variant_it != output_variants.end()) {
                                if ((last_key_variant_it->second->part_of_speech == PART_OF_SPEECH_NOUN &&
                                        it->second->part_of_speech == PART_OF_SPEECH_ADJECTIVE) ||
                                    (last_key_variant_it->second->component() == COMPONENT_OBJECT &&
                                        it->second->component() == COMPONENT_VERB)) {
                                    output_variants.insert(last_key_variant_it, *it);
                                    output_variants.erase(it + 1);
                                    done = false;
                                    break;
                                }
                            }

                            if (it->second->component() &&
                                (last_key_variant_it == output_variants.end() ||
                                    last_key_variant_it->second->component() != it->second->component())) {
                                last_key_variant_it = it;
                            }
                        }
                    } while (!done);
                }

                std::string output_sentence;

                for (size_t i = 0; i < output_variants.size(); ++i) {
                    if (i) output_sentence.push_back(' ');
                    output_sentence += output_variants[i].second->english_equivalent(output_variants[i].first);
                }

                std::vector<std::string> split_output_sentence = pw::string::split(output_sentence, ' ');
                split_output_sentence.erase(std::unique(split_output_sentence.begin(), split_output_sentence.end()), split_output_sentence.end());

                output_sentence.clear();
                for (size_t i = 0; i < split_output_sentence.size(); ++i) {
                    if (i) output_sentence.push_back(' ');
                    output_sentence += split_output_sentence[i];
                }

                return pw::HTTPResponse(200, output_sentence, {{"Content-Type", "text/plain"}});
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
