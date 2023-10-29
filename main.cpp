#include "Polyweb/polyweb.hpp"
#include "Polyweb/string.hpp"
#include "json.hpp"
#include <boost/process.hpp>
#include <cctype>
#include <iostream>
#include <limits>
#include <string>

using nlohmann::json;
namespace bp = boost::process;

int main() {
    pn::init();
    pn::UniqueSock<pw::Server> server;

    server->route("/word_info",
        pw::HTTPRoute {
            [](const pw::Connection&, const pw::HTTPRequest& req, void*) {
                pw::QueryParameters::map_type::const_iterator word_it;
                if ((word_it = req.query_parameters->find("word")) == req.query_parameters->end()) {
                    return pw::HTTPResponse::make_basic(400);
                }

                json resp = json::array();

                bp::ipstream out;
                bp::child words("./bin/words", word_it->second, bp::start_dir("whitakers-words"), bp::std_out > out);

                for (json word_variant; ; word_variant.clear()) {
                    std::string word, part_of_speech;
                    out >> word >> part_of_speech;
                    if (resp.empty()) {
                        if ((word == "Two" && part_of_speech == "words") || (word == word_it->second && part_of_speech == "========")) { // Check if Latin word wasn't found
                            return pw::HTTPResponse::make_basic(404);
                        }
                    } else if (word != resp[0]["word"]) {
                        break; // Variants are over
                    }
                    word_variant["word"] = word;
                    word_variant["part_of_speech"] = part_of_speech;

                    int unknown;
                    switch (part_of_speech[0]) {
                    case 'N': {
                        int declension;
                        std::string casus, plurality, gender;
                        out >> declension >> unknown >> casus >> plurality >> gender;
                        word_variant["declension"] = declension;
                        word_variant["casus"] = casus;
                        word_variant["plural"] = plurality[0] == 'P';
                        word_variant["gender"] = gender;
                        break;
                    }

                    case 'V': {
                        int conjugation, number;
                        std::string tense, voice, mood, plurality;
                        out >> conjugation >> unknown >> tense >> voice >> mood >> number >> plurality;
                        word_variant["conjugation"] = conjugation;
                        word_variant["tense"] = tense;
                        word_variant["voice"] = voice;
                        word_variant["mood"] = mood;
                        word_variant["number"] = number;
                        word_variant["plural"] = plurality[0] == 'P';
                        break;
                    }

                    case 'A': {
                        int declension;
                        std::string casus, plurality, gender;
                        out >> declension >> unknown >> casus >> plurality >> gender;
                        word_variant["declension"] = declension;
                        word_variant["casus"] = casus;
                        word_variant["plural"] = plurality[0] == 'P';
                        word_variant["gender"] = gender;
                        break;
                    }
                    }

                    resp.push_back(word_variant);
                    out.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Read the rest of the line
                }

                std::string base_english_word;
                out.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Read the rest of the line
                for (char c; out.get(c) && !ispunct(c);) {
                    base_english_word.push_back(c);
                }
                pw::string::trim_right(base_english_word);

                for (auto& word_variant : resp) {
                    std::string english_word = base_english_word;
                    switch(word_variant["part_of_speech"][0].get<char>()) {
                    case 'N': {
                        if (word_variant["casus"] == "GEN") {
                            english_word.insert(0, "of ");
                        } else if (word_variant["casus"] == "DAT") {
                            english_word.insert(0, "to/for ");
                        }

                        if (word_variant["plural"]) {
                            switch (english_word.back()) {
                            case 'a':
                            case 'i':
                            case 'o':
                            case 'u':
                                english_word.push_back('e');
                                break;

                            case 'y':
                                english_word.pop_back();
                                english_word += "ie";
                                break;
                            }
                            english_word.push_back('s');
                        }

                        break;
                    }

                    case 'V': {
                        switch (word_variant["number"].get<int>()) {
                        case 1:
                            if (word_variant["tense"] == "IMPF") {
                                english_word.insert(0, word_variant["plural"] ? "were " : "was ");
                            }
                            english_word.insert(0, word_variant["plural"] ? "we " : "I ");
                            break;

                        case 2:
                            if (word_variant["tense"] == "IMPF") {
                                english_word.insert(0, "were ");
                            }
                            english_word.insert(0, word_variant["plural"] ? "y'all " : "you ");
                            break;

                        case 3:
                            if (word_variant["tense"] == "IMPF") {
                                english_word.insert(0, "were ");
                            }
                            english_word.insert(0, "they ");
                            break;
                        }

                        if (word_variant["tense"] == "IMPF") {
                            if (english_word.back() == 'e') {
                                english_word.pop_back();
                            }
                            english_word += "ing";
                        } else if (word_variant["tense"] == "PERF") {
                            if (english_word.back() == 'e') {
                                english_word.push_back('d');
                            } else {
                                english_word += "ed";
                            }
                        } else if (word_variant["tense"] == "PLUP") {
                            english_word.insert(0, "had ");
                            if (english_word.back() == 'e') {
                                english_word.push_back('d');
                            } else {
                                english_word += "ed";
                            }
                        }

                        break;
                    }
                    }

                    word_variant["english_word"] = english_word;
                }

                return pw::HTTPResponse(200, resp.dump(), {{"Content-Type", "application/json"}});
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
