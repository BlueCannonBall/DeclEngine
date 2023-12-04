#include "Polyweb/polyweb.hpp"
#include "dictionary.hpp"
#include "json.hpp"
#include "words.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

using nlohmann::json;

int main(int argc, char* argv[]) {
    std::string port = argc >= 2 ? argv[1] : "8000";

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
                    std::sort(word.begin(), word.end(), [](const auto& a, const auto& b) {
                        bool a_has_upper = std::find_if(a.english_base.begin(), a.english_base.end(), isupper) != a.english_base.end();
                        bool b_has_upper = std::find_if(b.english_base.begin(), b.english_base.end(), isupper) != b.english_base.end();
                        if (a_has_upper == b_has_upper) {
                            return a.english_base.size() < b.english_base.size();
                        } else {
                            return b_has_upper;
                        }
                    });

                    json resp;
                    for (const auto& variant : word) {
                        json json_variant = {
                            {"forms", json::array()},
                            {"english_base", variant.english_base},
                        };

                        std::transform(variant.forms.begin(), variant.forms.end(), std::back_inserter(json_variant["forms"]), [&variant](const auto& form) {
                            json ret = form->to_json();
                            ret["english_equivalent"] = form->english_equivalent(variant.english_base);
                            return ret;
                        });

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

                for (auto string_word_it = split_input_sentence.begin(); string_word_it != split_input_sentence.end();) {
                    std::string beginning_punctuation;
                    std::string ending_punctuation;
                    for (size_t i = 0; i < string_word_it->size() && ispunct((*string_word_it)[i]); ++i) {
                        beginning_punctuation.push_back((*string_word_it)[i]);
                    }
                    for (size_t i = string_word_it->size(); i-- > 0 && ispunct((*string_word_it)[i]);) {
                        ending_punctuation.insert(ending_punctuation.begin(), (*string_word_it)[i]);
                    }
                    std::string stripped_word = string_word_it->substr(beginning_punctuation.size(), string_word_it->size() - beginning_punctuation.size() - ending_punctuation.size());

                    std::vector<WordVariant> word;
                    if (!query_dictionary(stripped_word, word)) {
                        if (pw::string::ends_with(stripped_word, "que")) {
                            string_word_it->erase(string_word_it->size() - ending_punctuation.size() - 3, 3);
                            string_word_it = std::next(split_input_sentence.insert(string_word_it, "et"));
                            input_words.push_back({
                                {
                                    .forms = {std::make_shared<Conjunction>()},
                                    .english_base = "and",
                                },
                            });
                            continue;
                        } else if (pw::string::ends_with(stripped_word, "cum")) {
                            string_word_it->erase(string_word_it->size() - ending_punctuation.size() - 3, 3);
                            string_word_it = std::next(split_input_sentence.insert(string_word_it, "cum"));
                            input_words.push_back({
                                {
                                    .forms = {std::make_shared<Preposition>(CASUS_ABLATIVE)},
                                    .english_base = "with",
                                },
                            });
                            continue;
                        } else if (pw::string::ends_with(stripped_word, "ne")) {
                            string_word_it->erase(string_word_it->size() - ending_punctuation.size() - 2, 2);
                            continue;
                        } else if (isupper(stripped_word.front())) {
                            input_words.push_back({
                                {
                                    .forms = {
                                        std::make_shared<Noun>(0, CASUS_NOMINATIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_GENITIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_DATIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_ACCUSATIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_ABLATIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_VOCATIVE, false, GENDER_COMMON),
                                        std::make_shared<Noun>(0, CASUS_LOCATIVE, false, GENDER_COMMON),
                                    },
                                    .english_base = stripped_word,
                                },
                            });
                            ++string_word_it;
                            continue;
                        } else {
                            return pw::HTTPResponse::make_basic(400);
                        }
                    }

                    std::sort(word.begin(), word.end(), [](const auto& a, const auto& b) {
                        bool a_has_upper = std::find_if(a.english_base.begin(), a.english_base.end(), isupper) != a.english_base.end();
                        bool b_has_upper = std::find_if(b.english_base.begin(), b.english_base.end(), isupper) != b.english_base.end();
                        if (a_has_upper == b_has_upper) {
                            return a.english_base.size() < b.english_base.size();
                        } else {
                            return b_has_upper;
                        }
                    });
                    input_words.push_back(std::move(word));
                    ++string_word_it;
                }

                std::vector<std::pair<std::string, std::shared_ptr<WordForm>>>
                    output_forms;
                {
                    std::vector<std::pair<std::string, std::shared_ptr<WordForm>>> current_clause;
                    size_t subjects = 0;
                    size_t verbs = 0;
                    size_t objects = 0;
                    bool subject_plural = false;
                    for (size_t i = 0; i < input_words.size();) {
                        auto& word = input_words[i];

                        if (!current_clause.empty()) {
                            const WordForm* prev_form = current_clause.back().second.get();
                            switch (prev_form->part_of_speech) {
                            case PART_OF_SPEECH_PREPOSITION:
                                for (const auto& variant : word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == prev_form->get_casus()) { // Check for noun with matching case
                                            current_clause.push_back({variant.english_base, form});
                                            goto next_word;
                                        }
                                    }
                                }
                                if (input_words[i - 1].size() > 1 ||
                                    input_words[i - 1].front().forms.size() > 1) {
                                    goto prev_word;
                                }

                            case PART_OF_SPEECH_CONJUNCTION:
                                if (current_clause.size() >= 2) {
                                    for (const auto& variant : word) {
                                        const WordForm* prev_prev_form = current_clause[i - output_forms.size() - 2].second.get();
                                        for (const auto& form : variant.forms) {
                                            if (form->part_of_speech == prev_prev_form->part_of_speech &&
                                                form->get_casus() == prev_prev_form->get_casus()) {
                                                current_clause.push_back({variant.english_base, form});
                                                if (form->get_casus() == CASUS_NOMINATIVE) {
                                                    subject_plural = true;
                                                }
                                                goto next_word;
                                            }
                                        }
                                    }
                                    goto next_clause;
                                }
                                break;

                            default:
                                for (const auto& variant : word) {
                                    switch (prev_form->part_of_speech) {
                                    case PART_OF_SPEECH_NOUN:
                                    case PART_OF_SPEECH_PRONOUN:
                                    case PART_OF_SPEECH_PARTICIPLE:
                                        if ((subjects && objects) ||
                                            std::find_if(variant.forms.begin(), variant.forms.end(), [](const auto& form) {
                                                return form->is_noun_like() && (form->get_casus() == CASUS_NOMINATIVE || form->get_casus() == CASUS_ACCUSATIVE);
                                            }) == variant.forms.end()) {
                                            // Check for genetive noun
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() && form->get_casus() == CASUS_GENITIVE) {
                                                    current_clause.push_back({variant.english_base, form});
                                                    goto next_word;
                                                }
                                            }

                                            // Check for ablative noun
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() && form->get_casus() == CASUS_ABLATIVE) {
                                                    current_clause.push_back({variant.english_base, form});
                                                    goto next_word;
                                                }
                                            }

                                            // Check for dative noun
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() && form->get_casus() == CASUS_DATIVE) {
                                                    current_clause.push_back({variant.english_base, form});
                                                    goto next_word;
                                                }
                                            }

                                            // Check for locative noun
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() && form->get_casus() == CASUS_LOCATIVE) {
                                                    current_clause.push_back({variant.english_base, form});
                                                    goto next_word;
                                                }
                                            }
                                        }

                                        // Check for adjective
                                        for (const auto& form : variant.forms) {
                                            Adjective* adjective;
                                            if ((adjective = dynamic_cast<Adjective*>(form.get())) &&
                                                adjective->casus == prev_form->get_casus() &&
                                                adjective->plural == prev_form->is_plural() &&
                                                adjective->gender == prev_form->get_gender()) {
                                                current_clause.push_back({variant.english_base, form});
                                                goto next_word;
                                            }
                                        }

                                        break;

                                    case PART_OF_SPEECH_ADJECTIVE:
                                        for (const auto& form : variant.forms) {
                                            Adjective* adjective;
                                            if ((adjective = dynamic_cast<Adjective*>(form.get())) && adjective->plural == prev_form->is_plural()) {
                                                current_clause.push_back({variant.english_base, form});
                                                goto next_word;
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_ADVERB:
                                        // Check for conjunction
                                        for (const auto& form : variant.forms) {
                                            if (form->part_of_speech == PART_OF_SPEECH_CONJUNCTION) {
                                                current_clause.push_back({variant.english_base, form});
                                                goto next_word;
                                            }
                                        }

                                        // Check for verb
                                        for (const auto& form : variant.forms) {
                                            if (form->part_of_speech == PART_OF_SPEECH_VERB && (!subjects || subjects < verbs || form->is_plural() == subject_plural)) { // Check for verb with matching number
                                                current_clause.push_back({variant.english_base, form});
                                                ++verbs;
                                                goto next_word;
                                            }
                                        }

                                        break;

                                    default:
                                        break;
                                    }
                                }
                                break;
                            }
                        }

                        // Check for preposition
                        for (const auto& variant : word) {
                            for (const auto& form : variant.forms) {
                                if (form->part_of_speech == PART_OF_SPEECH_PREPOSITION) {
                                    current_clause.push_back({variant.english_base, form});
                                    goto next_word;
                                }
                            }
                        }

                        // Check for adverb
                        for (const auto& variant : word) {
                            for (const auto& form : variant.forms) {
                                if (form->part_of_speech == PART_OF_SPEECH_ADVERB) {
                                    current_clause.push_back({variant.english_base, form});
                                    goto next_word;
                                }
                            }
                        }

                        // Check for subject
                        if (subjects <= objects) {
                            for (const auto& variant : word) {
                                for (const auto& form : variant.forms) {
                                    if (form->is_noun_like() && form->get_casus() == CASUS_NOMINATIVE) {
                                        current_clause.push_back({variant.english_base, form});
                                        ++subjects;
                                        subject_plural = form->is_plural();
                                        goto next_word;
                                    }
                                }
                            }
                        }

                        // Check for object
                        for (const auto& variant : word) {
                            for (const auto& form : variant.forms) {
                                if ((form->is_noun_like() || form->part_of_speech == PART_OF_SPEECH_ADJECTIVE) &&
                                    (form->get_casus() == CASUS_ACCUSATIVE ||
                                        (form->part_of_speech == PART_OF_SPEECH_ADJECTIVE && form->get_casus() == CASUS_NOMINATIVE))) {
                                    current_clause.push_back({variant.english_base, form});
                                    ++objects;
                                    goto next_word;
                                }
                            }
                        }

                        // Check for verb
                        for (const auto& variant : word) {
                            for (const auto& form : variant.forms) {
                                if (form->part_of_speech == PART_OF_SPEECH_VERB && (!subjects || subjects < verbs || form->is_plural() == subject_plural)) { // Check for verb with matching number
                                    current_clause.push_back({variant.english_base, form});
                                    ++verbs;
                                    goto next_word;
                                }
                            }
                        }

                        current_clause.push_back({word.front().english_base, word.front().forms.front()});

                    next_word:
                        if (ispunct(split_input_sentence[i++].back())) {
                            goto next_clause;
                        }
                        continue;

                    prev_word:
                        --i;
                        for (auto variant_it = input_words[i].begin(); variant_it != input_words[i].end(); ++variant_it) {
                            decltype(WordVariant::forms)::iterator form_it;
                            if ((form_it = std::find(variant_it->forms.begin(), variant_it->forms.end(), current_clause.back().second)) != variant_it->forms.end()) {
                                variant_it->forms.erase(form_it);
                                if (!variant_it->is_valid()) {
                                    input_words[i].erase(variant_it);
                                }
                                break;
                            }
                        }
                        current_clause.pop_back();
                        continue;

                    next_clause:
                        output_forms.reserve(output_forms.size() + current_clause.size());
                        std::move(current_clause.begin(), current_clause.end(), std::back_inserter(output_forms));
                        current_clause.clear();
                        subjects = 0;
                        verbs = 0;
                        objects = 0;
                        subject_plural = false;
                    }
                    if (!current_clause.empty()) {
                        output_forms.reserve(output_forms.size() + current_clause.size());
                        std::move(current_clause.begin(), current_clause.end(), std::back_inserter(output_forms));
                    }
                }

                // Reorder words
                bool done;
                do {
                    done = true;
                    for (size_t i = 1; i < output_forms.size(); ++i) {
                        if (output_forms[i].second->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                            output_forms[i - 1].second->is_noun_like() &&
                            output_forms[i].second->get_casus() == output_forms[i - 1].second->get_casus() &&
                            output_forms[i].second->is_plural() == output_forms[i - 1].second->is_plural() &&
                            output_forms[i].second->get_gender() == output_forms[i - 1].second->get_gender()) {
                            std::swap(output_forms[i], output_forms[i - 1]);
                            done = false;
                            break;
                        }
                    }
                } while (!done);

                std::string output_sentence;
                for (size_t i = 0; i < output_forms.size(); ++i) {
                    std::string beginning_punctuation;
                    std::string ending_punctuation;
                    for (size_t j = 0; j < split_input_sentence[i].size() && ispunct(split_input_sentence[i][j]); ++j) {
                        beginning_punctuation.push_back(split_input_sentence[i][j]);
                    }
                    for (size_t j = split_input_sentence[i].size(); j-- > 0 && ispunct(split_input_sentence[i][j]);) {
                        ending_punctuation.insert(ending_punctuation.begin(), split_input_sentence[i][j]);
                    }

                    if (i) {
                        output_sentence += "<SEP>";
                    }
                    if (output_forms[i].second->is_noun_like()) {
                        switch (output_forms[i].second->get_casus()) {
                        case CASUS_NOMINATIVE: output_sentence += "<NOM>"; break;
                        case CASUS_GENITIVE: output_sentence += "<GEN>"; break;
                        case CASUS_DATIVE: output_sentence += "<DAT>"; break;
                        case CASUS_ACCUSATIVE: output_sentence += "<ACC>"; break;
                        case CASUS_ABLATIVE: output_sentence += "<ABL>"; break;
                        case CASUS_VOCATIVE: output_sentence += "<VOC>"; break;
                        case CASUS_LOCATIVE: output_sentence += "<LOC>"; break;
                        default: throw std::logic_error("Invalid case");
                        }
                    }
                    output_sentence += beginning_punctuation + output_forms[i].second->english_equivalent(output_forms[i].first) + ending_punctuation;
                }

                // std::vector<std::string> split_output_sentence = pw::string::split(output_sentence, ' ');
                // split_output_sentence.erase(std::unique(split_output_sentence.begin(), split_output_sentence.end()), split_output_sentence.end());

                // output_sentence = split_output_sentence.front();
                // for (size_t i = 1; i < split_output_sentence.size(); ++i) {
                //     output_sentence.push_back(' ');
                //     output_sentence += split_output_sentence[i];
                // }

                return pw::HTTPResponse(200, output_sentence, {{"Content-Type", "text/plain"}});
            },
        });

    if (server->bind("0.0.0.0", port) == PN_ERROR) {
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
