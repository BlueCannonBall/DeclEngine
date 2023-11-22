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
                            json json_form = form->to_json();
                            json_form["english_equivalent"] = form->english_equivalent(variant.english_base);
                            json_variant["forms"].push_back(std::move(json_form));
                        }

                        resp.push_back(json_variant);
                    }

                    return pw::HTTPResponse(200, resp.dump(), {{"Content-Type", "application/json"}});
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
                for (auto string_word : split_input_sentence) {
                    std::vector<WordVariant> word;

                    if (pw::string::ends_with(string_word, "que")) {
                        string_word.erase(string_word.size() - 3);

                        query_dictionary("que", word);
                        std::sort(word.begin(), word.end(), [](const auto& a, const auto& b) {
                            return a.english_base.size() < b.english_base.size();
                        });
                        input_words.push_back(std::move(word));
                    }

                    if (query_dictionary(string_word, word)) {
                        std::sort(word.begin(), word.end(), [](const auto& a, const auto& b) {
                            return a.english_base.size() < b.english_base.size();
                        });
                        input_words.push_back(std::move(word));
                    }
                }

                std::vector<std::pair<std::string, std::shared_ptr<WordForm>>> output_forms;
                {
                    std::vector<std::shared_ptr<Noun>> subjects;
                    std::vector<std::shared_ptr<Verb>> verbs;
                    std::vector<std::shared_ptr<Noun>> objects;
                    for (const auto& word : input_words) {
                        for (const auto& variant : word) {
                            if (!output_forms.empty()) {
                                WordForm* previous_form = output_forms.back().second.get();
                                switch (previous_form->part_of_speech) {
                                case PART_OF_SPEECH_PREPOSITION: {
                                    for (const auto& form : variant.forms) {
                                        auto noun = dynamic_cast<Noun*>(form.get());
                                        if (noun && noun->casus == previous_form->get_casus()) { // Check for noun with matching case
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                    break;
                                }

                                case PART_OF_SPEECH_NOUN: {
                                    for (const auto& form : variant.forms) {
                                        auto adjective = dynamic_cast<Adjective*>(form.get());
                                        if (adjective && adjective->plural == previous_form->is_plural()) { // Check for adjective with matching number
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                    break;
                                }

                                case PART_OF_SPEECH_ADJECTIVE: {
                                    for (const auto& form : variant.forms) {
                                        auto adjective = dynamic_cast<Adjective*>(form.get());
                                        if (adjective && adjective->plural == previous_form->is_plural()) { // Check for adjective with matching number
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                    break;
                                }

                                case PART_OF_SPEECH_ADVERB:
                                    // Check for conjunction
                                    for (const auto& form : variant.forms) {
                                        if (form->part_of_speech == PART_OF_SPEECH_CONJUNCTION) {
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }

                                    // Check for verb
                                    for (const auto& form : variant.forms) {
                                        auto verb = dynamic_cast<Verb*>(form.get());
                                        if (verb && (subjects.size() <= verbs.size() || verb->plural == subjects.back()->plural)) { // Check for verb with matching number
                                            verbs.push_back(std::static_pointer_cast<Verb>(form));
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }

                                    break;

                                default:
                                    goto skip_narrowing;
                                }
                            } else {
                            skip_narrowing:
                                switch (variant.part_of_speech()) {
                                case PART_OF_SPEECH_NOUN:
                                case PART_OF_SPEECH_PRONOUN:
                                    // Check for subject
                                    if (subjects.size() <= objects.size()) {
                                        for (const auto& form : variant.forms) {
                                            if (form->get_casus() == CASUS_NOMINATIVE) {
                                                subjects.push_back(std::static_pointer_cast<Noun>(form));
                                                output_forms.push_back({variant.english_base, form});
                                                goto next_word;
                                            }
                                        }
                                    }

                                    // Check for object
                                    for (const auto& form : variant.forms) {
                                        if (form->get_casus() == CASUS_ACCUSATIVE) {
                                            objects.push_back(std::static_pointer_cast<Noun>(form));
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }

                                    break;

                                case PART_OF_SPEECH_VERB:
                                    for (const auto& form : variant.forms) {
                                        if (subjects.size() <= objects.size() || form->is_plural() == subjects.back()->plural) { // Check for verb with matching number
                                            verbs.push_back(std::static_pointer_cast<Verb>(form));
                                            output_forms.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                    break;

                                case PART_OF_SPEECH_ADVERB:
                                    output_forms.push_back({variant.english_base, variant.forms.front()});
                                    goto next_word;

                                default:
                                    break;
                                }
                            }
                        }

                        output_forms.push_back({word.front().english_base, word.front().forms.front()});

                    next_word:;
                    }
                }

                // Reorder words
                bool done;
                do {
                    done = true;
                    for (size_t i = 1; i < output_forms.size(); ++i) {
                        if (output_forms[i].second->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                            output_forms[i - 1].second->part_of_speech == PART_OF_SPEECH_NOUN) {
                            auto adjective = (Adjective*) output_forms[i].second.get();
                            auto noun = (Noun*) output_forms[i - 1].second.get();
                            if (adjective->casus == noun->casus) {
                                std::swap(output_forms[i], output_forms[i - 1]);
                                done = false;
                                break;
                            }
                        }
                    }
                } while (!done);

                std::string output_sentence = output_forms.front().second->english_equivalent(output_forms.front().first);
                for (size_t i = 1; i < output_forms.size(); ++i) {
                    output_sentence.push_back(' ');
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
