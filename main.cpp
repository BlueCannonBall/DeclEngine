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

                std::vector<WordVariant> word;
                if (query_dictionary(word_it->second, word)) {
                    json resp;
                    for (const auto& variant : word) {
                        json json_variant = {
                            {"forms", json::array()},
                            {"english_base", variant.english_base},
                        };

                        for (const auto& form : variant.forms) {
                            json json_form;

                            switch (form->part_of_speech) {
                            case PART_OF_SPEECH_NOUN:
                            case PART_OF_SPEECH_PRONOUN: {
                                if (form->part_of_speech == PART_OF_SPEECH_NOUN) {
                                    json_form["part_of_speech"] = "noun";
                                } else if (form->part_of_speech == PART_OF_SPEECH_PRONOUN) {
                                    json_form["part_of_speech"] = "pronoun";
                                } else {
                                    throw std::logic_error("Invalid part of speech");
                                }
                                auto noun = (Noun*) form.get();

                                switch (noun->casus) {
                                case CASUS_NOMINATIVE: json_form["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_form["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_form["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_form["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_form["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_form["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_form["casus"] = "locative"; break;
                                }

                                json_form["plural"] = noun->plural;

                                switch (noun->gender) {
                                case GENDER_MASCULINE: json_form["gender"] = "masculine"; break;
                                case GENDER_FEMININE: json_form["gender"] = "feminine"; break;
                                case GENDER_NEUTER: json_form["gender"] = "neuter"; break;
                                case GENDER_COMMON: json_form["gender"] = "common"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_VERB: {
                                json_form["part_of_speech"] = "verb";
                                auto verb = (Verb*) form.get();

                                switch (verb->tense) {
                                case TENSE_PRESENT: json_form["tense"] = "present"; break;
                                case TENSE_IMPERFECT: json_form["tense"] = "imperfect"; break;
                                case TENSE_PERFECT: json_form["tense"] = "perfect"; break;
                                case TENSE_PLUPERFECT: json_form["tense"] = "pluperfect"; break;
                                case TENSE_FUTURE: json_form["tense"] = "future"; break;
                                case TENSE_FUTURE_PERFECT: json_form["tense"] = "future_perfect"; break;
                                }

                                switch (verb->voice) {
                                case VOICE_ACTIVE: json_form["voice"] = "active"; break;
                                case VOICE_PASSIVE: json_form["voice"] = "passive"; break;
                                }

                                switch (verb->mood) {
                                case MOOD_INDICATIVE: json_form["mood"] = "indicative"; break;
                                case MOOD_SUBJUNCTIVE: json_form["mood"] = "subjunctive"; break;
                                case MOOD_IMPERATIVE: json_form["mood"] = "imperative"; break;
                                case MOOD_INFINITIVE: json_form["mood"] = "infinitive"; break;
                                }

                                json_form["plural"] = verb->plural;

                                break;
                            }

                            case PART_OF_SPEECH_PARTICIPLE: {
                                json_form["part_of_speech"] = "participle";
                                auto participle = (Participle*) form.get();

                                switch (participle->casus) {
                                case CASUS_NOMINATIVE: json_form["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_form["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_form["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_form["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_form["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_form["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_form["casus"] = "locative"; break;
                                }

                                json_form["plural"] = participle->plural;

                                switch (participle->gender) {
                                case GENDER_MASCULINE: json_form["gender"] = "masculine"; break;
                                case GENDER_FEMININE: json_form["gender"] = "feminine"; break;
                                case GENDER_NEUTER: json_form["gender"] = "neuter"; break;
                                case GENDER_COMMON: json_form["gender"] = "common"; break;
                                }

                                switch (participle->tense) {
                                case TENSE_PRESENT: json_form["tense"] = "present"; break;
                                case TENSE_IMPERFECT: json_form["tense"] = "imperfect"; break;
                                case TENSE_PERFECT: json_form["tense"] = "perfect"; break;
                                case TENSE_PLUPERFECT: json_form["tense"] = "pluperfect"; break;
                                case TENSE_FUTURE: json_form["tense"] = "future"; break;
                                case TENSE_FUTURE_PERFECT: json_form["tense"] = "future_perfect"; break;
                                }

                                switch (participle->voice) {
                                case VOICE_ACTIVE: json_form["voice"] = "active"; break;
                                case VOICE_PASSIVE: json_form["voice"] = "passive"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_SUPINE: {
                                json_form["part_of_speech"] = "supine";
                                auto supine = (Supine*) form.get();

                                switch (supine->casus) {
                                case CASUS_NOMINATIVE: json_form["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_form["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_form["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_form["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_form["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_form["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_form["casus"] = "locative"; break;
                                }

                                json_form["plural"] = supine->plural;

                                switch (supine->gender) {
                                case GENDER_MASCULINE: json_form["gender"] = "masculine"; break;
                                case GENDER_FEMININE: json_form["gender"] = "feminine"; break;
                                case GENDER_NEUTER: json_form["gender"] = "neuter"; break;
                                case GENDER_COMMON: json_form["gender"] = "common"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_ADJECTIVE: {
                                json_form["part_of_speech"] = "adjective";
                                auto adjective = (Adjective*) form.get();

                                switch (adjective->casus) {
                                case CASUS_NOMINATIVE: json_form["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_form["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_form["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_form["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_form["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_form["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_form["casus"] = "locative"; break;
                                }

                                json_form["plural"] = adjective->plural;

                                switch (adjective->gender) {
                                case GENDER_MASCULINE: json_form["gender"] = "masculine"; break;
                                case GENDER_FEMININE: json_form["gender"] = "feminine"; break;
                                case GENDER_NEUTER: json_form["gender"] = "neuter"; break;
                                case GENDER_COMMON: json_form["gender"] = "common"; break;
                                }

                                switch (adjective->degree) {
                                case DEGREE_POSITIVE: json_form["degree"] = "positive"; break;
                                case DEGREE_COMPARATIVE: json_form["degree"] = "comparative"; break;
                                case DEGREE_SUPERLATIVE: json_form["degree"] = "superlative"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_ADVERB: {
                                json_form["part_of_speech"] = "adverb";
                                auto adverb = (Adverb*) form.get();

                                switch (adverb->degree) {
                                case DEGREE_POSITIVE: json_form["degree"] = "positive"; break;
                                case DEGREE_COMPARATIVE: json_form["degree"] = "comparative"; break;
                                case DEGREE_SUPERLATIVE: json_form["degree"] = "superlative"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_CONJUNCTION: {
                                json_form["part_of_speech"] = "conjunction";
                                break;
                            }

                            case PART_OF_SPEECH_PREPOSITION: {
                                json_form["part_of_speech"] = "preposition";
                                auto preposition = (Preposition*) form.get();

                                switch (preposition->casus) {
                                case CASUS_NOMINATIVE: json_form["casus"] = "nominative"; break;
                                case CASUS_GENITIVE: json_form["casus"] = "genitive"; break;
                                case CASUS_DATIVE: json_form["casus"] = "dative"; break;
                                case CASUS_ACCUSATIVE: json_form["casus"] = "accusative"; break;
                                case CASUS_ABLATIVE: json_form["casus"] = "ablative"; break;
                                case CASUS_VOCATIVE: json_form["casus"] = "vocative"; break;
                                case CASUS_LOCATIVE: json_form["casus"] = "locative"; break;
                                }

                                break;
                            }

                            case PART_OF_SPEECH_INTERJECTION: {
                                json_form["part_of_speech"] = "interjection";
                                break;
                            }
                            }

                            json_form["english_equivalent"] = form->english_equivalent(variant.english_base);
                            json_variant["forms"].push_back(json_form);
                        }

                        resp.push_back(json_variant);
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
                std::vector<std::vector<WordVariant>> input_words;
                for (const auto& string_word : split_input_sentence) {
                    std::vector<WordVariant> word;
                    if (query_dictionary(string_word, word)) {
                        std::sort(word.begin(), word.end(), [](const auto& a, const auto& b) {
                            return a.english_base.size() < b.english_base.size();
                        });
                        input_words.push_back(std::move(word));
                    }
                }

                // Eliminate impossible forms
                // for (size_t i = 0; i < input_words.size() - 1; ++i) {
                //     size_t j = i + 1;
                //     for (const auto& form : input_words[i].forms) {
                //         if (auto preposition = dynamic_cast<Preposition*>(form.get())) {
                //             input_words[j].forms.erase(std::remove_if(input_words[j].forms.begin(), input_words[j].forms.end(), [preposition](const auto& form) {
                //                 Noun* noun;
                //                 return (noun = dynamic_cast<Noun*>(form.get())) && noun->casus != preposition->casus;
                //             }),
                //                 input_words[j].forms.end());
                //         }
                //     }
                // }

                // std::vector<std::vector<WordInfo>> word_groups;
                // std::vector<WordInfo> group;
                // for (const auto& word : input_words) {
                //     if (group.empty()) {
                //         group.push_back(word);
                //     } else {

                //         switch (group.front().part_of_speech()) {
                //         default:
                //             break;
                //         }
                //     }
                //     // word_groups.push_back(std::move(group));
                // }

                std::vector<std::pair<std::string, std::shared_ptr<WordForm>>> output_forms;
                {
                    std::vector<std::shared_ptr<Noun>> subjects;
                    std::vector<std::shared_ptr<Verb>> verbs;
                    std::vector<std::shared_ptr<Noun>> objects;
                    for (const auto& word : input_words) {
                        for (const auto& variant : word) {
                            switch (variant.part_of_speech()) {
                            case PART_OF_SPEECH_NOUN:
                            case PART_OF_SPEECH_PRONOUN:
                                if (!output_forms.empty() && output_forms.back().second->part_of_speech == PART_OF_SPEECH_PREPOSITION) {
                                    // Check for preposition + accusative
                                    for (const auto& form : variant.forms) {
                                        auto noun = (Noun*) form.get();
                                        if (noun->casus == CASUS_ACCUSATIVE) {
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }

                                    // Check for preposition + ablative
                                    for (const auto& form : variant.forms) {
                                        auto noun = (Noun*) form.get();
                                        if (noun->casus == CASUS_ABLATIVE) {
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                }

                                // Check for subject
                                if (subjects.size() <= objects.size()) {
                                    for (const auto& form : variant.forms) {
                                        auto noun = (Noun*) form.get();
                                        if (noun->casus == CASUS_NOMINATIVE) {
                                            subjects.push_back(std::static_pointer_cast<Noun>(form));
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                }

                                // Check for object
                                for (const auto& form : variant.forms) {
                                    auto noun = (Noun*) form.get();
                                    if (noun->casus == CASUS_ACCUSATIVE) {
                                        objects.push_back(std::static_pointer_cast<Noun>(form));
                                        output_forms.push_back({variant.english_base, form});
                                        goto next_word;
                                    }
                                }

                                break;

                            case PART_OF_SPEECH_ADJECTIVE:
                                for (const auto& form : variant.forms) {
                                    auto adjective = (Adjective*) form.get();
                                    if (output_forms.empty() || adjective->plural == ((Noun*) output_forms.back().second.get())->plural) { // Check for adjective with matching number
                                        output_forms.push_back({variant.english_base, form});
                                        goto next_word;
                                    }
                                }
                                break;

                            case PART_OF_SPEECH_VERB:
                                if (output_forms.empty() || output_forms.back().second->part_of_speech != PART_OF_SPEECH_PREPOSITION) {
                                    for (const auto& form : variant.forms) {
                                        auto verb = (Verb*) form.get();
                                        if (subjects.empty() || verb->plural == subjects.back()->plural) { // Check for verb with matching number
                                            verbs.push_back(std::static_pointer_cast<Verb>(form));
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                }
                                break;

                            default:
                                break;
                            }
                        }

                        output_forms.push_back({word.front().english_base, word.front().forms.front()});

                    next_word:;
                    }
                }

                // Reorder words
                // {
                //     bool done;
                //     do {
                //         done = true;
                //         decltype(output_forms)::iterator last_key_form_it = output_forms.end();
                //         for (auto it = output_forms.begin(); it != output_forms.end(); ++it) {
                //             if (last_key_form_it != output_forms.end()) {
                //                 if ((last_key_form_it->second->part_of_speech == PART_OF_SPEECH_NOUN &&
                //                         it->second->part_of_speech == PART_OF_SPEECH_ADJECTIVE) ||
                //                     (last_key_form_it->second->component() == COMPONENT_OBJECT &&
                //                         it->second->component() == COMPONENT_VERB)) {
                //                     output_forms.insert(last_key_form_it, *it);
                //                     output_forms.erase(it + 1);
                //                     done = false;
                //                     break;
                //                 }
                //             }

                //             if (it->second->component() &&
                //                 (last_key_form_it == output_forms.end() ||
                //                     last_key_form_it->second->component() != it->second->component())) {
                //                 last_key_form_it = it;
                //             }
                //         }
                //     } while (!done);
                // }

                std::string output_sentence;

                for (size_t i = 0; i < output_forms.size(); ++i) {
                    if (i) output_sentence.push_back(' ');
                    output_sentence += output_forms[i].second->english_equivalent(output_forms[i].first);
                }

                std::vector<std::string> split_output_sentence = pw::string::split(output_sentence, ' ');
                split_output_sentence.erase(std::unique(split_output_sentence.begin(), split_output_sentence.end()), split_output_sentence.end());

                output_sentence = split_output_sentence.front();
                for (size_t i = 1; i < split_output_sentence.size(); ++i) {
                    output_sentence.push_back(' ');
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
