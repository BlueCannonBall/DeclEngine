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

                for (auto string_word_it = split_input_sentence.begin(); string_word_it != split_input_sentence.end(); ++string_word_it) {
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
                            string_word_it = std::next(split_input_sentence.insert(string_word_it, "et"));
                            input_words.push_back({
                                {
                                    .forms = {std::make_shared<Conjunction>()},
                                    .english_base = "and",
                                },
                            });
                            stripped_word.erase(stripped_word.size() - 3);
                            if (!query_dictionary(stripped_word, word)) {
                                if (isupper(stripped_word.front())) {
                                    input_words.push_back({WordVariant::make_proper_noun(stripped_word)});
                                    continue;
                                } else {
                                    return pw::HTTPResponse(400);
                                }
                            }
                        } else if (pw::string::ends_with(stripped_word, "cum")) {
                            string_word_it = std::next(split_input_sentence.insert(string_word_it, "cum"));
                            input_words.push_back({
                                {
                                    .forms = {std::make_shared<Preposition>(CASUS_ABLATIVE)},
                                    .english_base = "with",
                                },
                            });
                            stripped_word.erase(stripped_word.size() - 3);
                            if (!query_dictionary(stripped_word, word)) {
                                if (isupper(stripped_word.front())) {
                                    input_words.push_back({WordVariant::make_proper_noun(stripped_word)});
                                    continue;
                                } else {
                                    return pw::HTTPResponse(400);
                                }
                            }
                        } else if (pw::string::ends_with(stripped_word, "ne")) {
                            stripped_word.erase(stripped_word.size() - 2);
                            if (!query_dictionary(stripped_word, word)) {
                                if (isupper(stripped_word.front())) {
                                    input_words.push_back({WordVariant::make_proper_noun(stripped_word)});
                                    continue;
                                } else {
                                    return pw::HTTPResponse(400);
                                }
                            } else {
                                for (auto variant_it = word.begin(); variant_it != word.end();) {
                                    variant_it->forms.erase(std::remove_if(variant_it->forms.begin(), variant_it->forms.end(), [](const auto& form) {
                                        return form->part_of_speech != PART_OF_SPEECH_VERB;
                                    }),
                                        variant_it->forms.end());
                                    if (!variant_it->is_valid()) {
                                        variant_it = word.erase(variant_it);
                                    } else {
                                        ++variant_it;
                                    }
                                }
                                if (word.empty()) {
                                    return pw::HTTPResponse(400);
                                }
                            }
                        } else if (isupper(stripped_word.front())) {
                            input_words.push_back({WordVariant::make_proper_noun(stripped_word)});
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
                }

                std::vector<std::pair<std::string, std::shared_ptr<WordForm>>> output_forms;
                {
                    // PHASE 1: RESOLVE GIVENS
                    output_forms.reserve(input_words.size());
                    for (const auto& word : input_words) {
                        if (word.size() == 1 && word.front().forms.size() == 1) {
                            output_forms.push_back({word.front().english_base, word.front().forms.front()});
                        } else {
                            output_forms.push_back({{}, nullptr});
                        }
                    }

                    // PHASE 2: RESOLVE UNKNOWNS
                    bool resolved;
                    do {
                        resolved = false;

                        // PHASE 2.1: RESOLVE UNKNOWNS USING KNOWN SURROUNDINGS
                        for (size_t i = 0; i < output_forms.size();) {
                            auto& current_form = output_forms[i];

                            if (!current_form.second) {
                                const auto& current_word = input_words[i];

                                if (i != 0 && output_forms[i - 1].second && !ispunct(split_input_sentence[i - 1].back())) {
                                    const auto& prev_form = output_forms[i - 1];
                                    switch (prev_form.second->part_of_speech) {
                                    case PART_OF_SPEECH_CONJUNCTION:
                                        if (i != 1 &&
                                            output_forms[i - 2].second &&
                                            !ispunct(split_input_sentence[i - 2].back()) &&
                                            (prev_form.first == "and" || prev_form.first == "or")) {
                                            const auto& prev_prev_form = output_forms[i - 2];
                                            switch (prev_prev_form.second->part_of_speech) {
                                            case PART_OF_SPEECH_NOUN:
                                            case PART_OF_SPEECH_PRONOUN:
                                            case PART_OF_SPEECH_PARTICIPLE:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->is_noun_like() &&
                                                            form->get_casus() == prev_prev_form.second->get_casus() &&
                                                            form->is_plural() == prev_prev_form.second->is_plural()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_VERB:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_VERB &&
                                                            form->is_plural() == prev_prev_form.second->is_plural()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADJECTIVE:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                                                            form->get_casus() == prev_prev_form.second->get_casus() &&
                                                            form->is_plural() == prev_prev_form.second->is_plural() &&
                                                            form->get_gender() == prev_prev_form.second->get_gender()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADVERB:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_ADVERB) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_PREPOSITION:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() && form->get_casus() == prev_form.second->get_casus()) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_NOUN:
                                    case PART_OF_SPEECH_PRONOUN:
                                    case PART_OF_SPEECH_PARTICIPLE:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                                                    form->get_casus() == prev_form.second->get_casus() &&
                                                    form->is_plural() == prev_form.second->is_plural() &&
                                                    form->get_gender() == prev_form.second->get_gender()) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_VERB:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->part_of_speech == PART_OF_SPEECH_ADVERB) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    default:
                                        break;
                                    }
                                }

                                if (i != output_forms.size() - 1 &&
                                    output_forms[i + 1].second &&
                                    !ispunct(split_input_sentence[i].back()) &&
                                    !ispunct(split_input_sentence[i + 1].back())) {
                                    const auto& next_form = output_forms[i + 1];
                                    switch (next_form.second->part_of_speech) {
                                    case PART_OF_SPEECH_CONJUNCTION:
                                        if ((next_form.first == "and" || next_form.first == "or") && i != output_forms.size() - 2 && output_forms[i + 2].second) {
                                            const auto& next_next_form = output_forms[i + 2];
                                            switch (next_next_form.second->part_of_speech) {
                                            case PART_OF_SPEECH_NOUN:
                                            case PART_OF_SPEECH_PRONOUN:
                                            case PART_OF_SPEECH_PARTICIPLE:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->is_noun_like() &&
                                                            form->get_casus() == next_next_form.second->get_casus() &&
                                                            form->is_plural() == next_next_form.second->is_plural()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_VERB:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_VERB &&
                                                            form->is_plural() == next_next_form.second->is_plural()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADJECTIVE:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                                                            form->get_casus() == next_next_form.second->get_casus() &&
                                                            form->is_plural() == next_next_form.second->is_plural() &&
                                                            form->get_gender() == next_next_form.second->get_gender()) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADVERB:
                                                for (const auto& variant : current_word) {
                                                    for (const auto& form : variant.forms) {
                                                        if (form->part_of_speech == PART_OF_SPEECH_ADVERB) {
                                                            current_form = {variant.english_base, form};
                                                            goto next_form;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_NOUN:
                                    case PART_OF_SPEECH_PRONOUN:
                                    case PART_OF_SPEECH_PARTICIPLE:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->part_of_speech == PART_OF_SPEECH_PREPOSITION &&
                                                    form->get_casus() == next_form.second->get_casus()) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_ADJECTIVE:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->is_noun_like() &&
                                                    form->get_casus() == next_form.second->get_casus() &&
                                                    form->is_plural() == next_form.second->is_plural() &&
                                                    form->get_gender() == next_form.second->get_gender()) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    case PART_OF_SPEECH_ADVERB:
                                        for (const auto& variant : current_word) {
                                            for (const auto& form : variant.forms) {
                                                if (form->part_of_speech == PART_OF_SPEECH_VERB) {
                                                    current_form = {variant.english_base, form};
                                                    goto next_form;
                                                }
                                            }
                                        }
                                        break;

                                    default:
                                        break;
                                    }
                                }
                            }

                            ++i;
                            continue;

                        next_form:
                            resolved = true;
                            ++i;
                        }

                        // PHASE 2.2: RESOLVE SETS OF UNKNOWNS USING COMMONALITIES
                        for (size_t i = 0; i < output_forms.size() - 1; ++i) {
                            auto& current_form = output_forms[i];
                            auto& next_form = output_forms[i + 1];
                            if (current_form.second) {
                                if (i &&
                                    current_form.second->part_of_speech == PART_OF_SPEECH_CONJUNCTION &&
                                    (current_form.first == "and" || current_form.first == "or") &&
                                    !output_forms[i - 1].second &&
                                    !next_form.second &&
                                    !ispunct(split_input_sentence[i - 1].back()) &&
                                    !ispunct(split_input_sentence[i].back())) {
                                    auto& prev_form = output_forms[i - 1];
                                    const auto& prev_word = input_words[i - 1];
                                    for (const auto& variant_a : prev_word) {
                                        for (const auto& form_a : variant_a.forms) {
                                            const auto& next_word = input_words[i + 1];
                                            switch (form_a->part_of_speech) {
                                            case PART_OF_SPEECH_NOUN:
                                            case PART_OF_SPEECH_PRONOUN:
                                            case PART_OF_SPEECH_PARTICIPLE:
                                                for (const auto& variant_b : next_word) {
                                                    for (const auto& form_b : variant_b.forms) {
                                                        if (form_b->is_noun_like() &&
                                                            form_a->get_casus() == form_b->get_casus() &&
                                                            form_a->is_plural() == form_b->is_plural()) {
                                                            prev_form = {variant_a.english_base, form_a};
                                                            next_form = {variant_b.english_base, form_b};
                                                            goto next_cycle;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_VERB:
                                                for (const auto& variant_b : next_word) {
                                                    for (const auto& form_b : variant_b.forms) {
                                                        if (form_b->part_of_speech == PART_OF_SPEECH_VERB &&
                                                            form_a->is_plural() == form_b->is_plural()) {
                                                            prev_form = {variant_a.english_base, form_a};
                                                            next_form = {variant_b.english_base, form_b};
                                                            goto next_cycle;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADJECTIVE:
                                                for (const auto& variant_b : next_word) {
                                                    for (const auto& form_b : variant_b.forms) {
                                                        if (form_b->part_of_speech == PART_OF_SPEECH_ADJECTIVE &&
                                                            form_a->get_casus() == form_b->get_casus() &&
                                                            form_a->is_plural() == form_b->is_plural() &&
                                                            form_a->get_gender() == form_b->get_gender()) {
                                                            prev_form = {variant_a.english_base, form_a};
                                                            next_form = {variant_b.english_base, form_b};
                                                            goto next_cycle;
                                                        }
                                                    }
                                                }
                                                break;

                                            case PART_OF_SPEECH_ADVERB:
                                                for (const auto& variant_b : next_word) {
                                                    for (const auto& form_b : variant_b.forms) {
                                                        if (form_b->part_of_speech == PART_OF_SPEECH_ADVERB) {
                                                            prev_form = {variant_a.english_base, form_a};
                                                            next_form = {variant_b.english_base, form_b};
                                                            goto next_cycle;
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                            }
                                        }
                                    }
                                }
                            } else if (!next_form.second) {
                                const auto& current_word = input_words[i];
                                for (const auto& variant_a : current_word) {
                                    for (const auto& form_a : variant_a.forms) {
                                        if (form_a->part_of_speech == PART_OF_SPEECH_PREPOSITION) {
                                            const auto& next_word = input_words[i + 1];
                                            for (const auto& variant_b : next_word) {
                                                for (const auto& form_b : variant_b.forms) {
                                                    if (form_b->is_noun_like() && form_a->get_casus() == form_b->get_casus()) {
                                                        current_form = {variant_a.english_base, form_a};
                                                        next_form = {variant_b.english_base, form_b};
                                                        goto next_cycle;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // PHASE 2.3: NAIVELY DISCOVER GENITIVES, DATIVES, AND ABLATIVES AFTER OTHER NOUN-LIKES
                        for (size_t i = 0; i < output_forms.size() - 1; ++i) {
                            const auto& current_form = output_forms[i];
                            auto& next_form = output_forms[i + 1];
                            if (current_form.second && current_form.second->is_noun_like() && !next_form.second) {
                                const auto& next_word = input_words[i + 1];

                                // Check for genitives
                                for (const auto& variant : next_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == CASUS_GENITIVE) {
                                            next_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }

                                // Check for datives
                                for (const auto& variant : next_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == CASUS_DATIVE) {
                                            next_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }

                                // Check for ablatives
                                for (const auto& variant : next_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == CASUS_ABLATIVE) {
                                            next_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }
                            }
                        }

                        // PHASE 2.4: NAIVELY DISCOVER SUBJECTS, OBJECTS, AND VERBS
                        for (size_t i = 0; i < output_forms.size(); ++i) {
                            auto& current_form = output_forms[i];
                            if (!current_form.second) {
                                const auto& current_word = input_words[i];

                                // Check for subjects
                                for (const auto& variant : current_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == CASUS_NOMINATIVE) {
                                            current_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }

                                // Check for objects
                                for (const auto& variant : current_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->is_noun_like() && form->get_casus() == CASUS_ACCUSATIVE) {
                                            current_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }

                                // Check for verbs
                                for (const auto& variant : current_word) {
                                    for (const auto& form : variant.forms) {
                                        if (form->part_of_speech == PART_OF_SPEECH_VERB) {
                                            current_form = {variant.english_base, form};
                                            goto next_cycle;
                                        }
                                    }
                                }
                            }
                        }

                        // PHASE 2.5: PICK THE TOP FORM FOR REMAINING UNKNOWNS
                        for (size_t i = 0; i < output_forms.size(); ++i) {
                            auto& current_form = output_forms[i];
                            if (!current_form.second) {
                                current_form = {input_words[i].front().english_base, input_words[i].front().forms.front()};
                                goto next_cycle;
                            }
                        }

                        continue;

                    next_cycle:
                        resolved = true;
                    } while (resolved);
                }

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
