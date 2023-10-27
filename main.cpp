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

                json resp;

                bp::ipstream out;
                bp::child words("./bin/words", word_it->second, bp::start_dir("whitakers-words"), bp::std_out > out);

                std::string word, part_of_speech;
                out >> word >> part_of_speech;
                resp["word"] = word;
                resp["part_of_speech"] = part_of_speech;

                int unknown;
                switch (part_of_speech[0]) {
                case 'N': {
                    int declension;
                    std::string casus, plurality, gender;
                    out >> declension >> unknown >> casus >> plurality >> gender;
                    resp["declension"] = declension;
                    resp["casus"] = casus;
                    resp["plural"] = plurality[0] == 'P';
                    resp["gender"] = gender;

                    std::string english_word;
                    out.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the first line
                    for (std::string line; std::getline(out, line, '\n');) {
                        pw::string::trim_right(line);
                        if (line.back() == ']') {
                            for (char c; out.get(c) && !ispunct(c);) {
                                english_word.push_back(c);
                            }
                            pw::string::trim_right(english_word);
                            break;
                        }
                    }

                    // Decline English word
                    if (plurality[0] == 'P') {
                        switch (english_word.back()) {
                        case 'a':
                        case 'e':
                        case 'i':
                        case 'o':
                        case 'u':
                            english_word.push_back('e');
                            break;
                        }
                        english_word.push_back('s');
                    }
                    resp["english_word"] = english_word;

                    break;
                }

                case 'V': {
                    int conjugation, number;
                    std::string tense, voice, mood, plurality;
                    out >> conjugation >> unknown >> tense >> voice >> mood >> number >> plurality;
                    resp["conjugation"] = conjugation;
                    resp["tense"] = tense;
                    resp["voice"] = voice;
                    resp["mood"] = mood;
                    resp["number"] = number;
                    resp["plural"] = plurality[0] == 'P';

                    std::string english_word;
                    out.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the first line
                    for (std::string line; std::getline(out, line, '\n');) {
                        pw::string::trim_right(line);
                        if (line.back() == ']') {
                            for (char c; out.get(c) && !ispunct(c);) {
                                english_word.push_back(c);
                            }
                            pw::string::trim_right(english_word);
                            break;
                        }
                    }

                    // Conjugate English word
                    if (tense == "IMPF") {
                        english_word.insert(0, (plurality[0] == 'S') ? "was " : "were ");
                        if (english_word.back() == 'e') {
                            english_word.pop_back();
                        }
                        english_word += "ing";
                    } else if (tense == "PERF") {
                        english_word += "ed";
                    } else if (tense == "PLUP") {
                        english_word.insert(0, "had ");
                        english_word += "ed";
                    }
                    resp["english_word"] = english_word;

                    break;
                }

                case 'A': {
                    int declension;
                    std::string casus, plurality, gender;
                    out >> declension >> unknown >> casus >> plurality >> gender;
                    resp["declension"] = declension;
                    resp["casus"] = casus;
                    resp["plural"] = plurality[0] == 'P';
                    resp["gender"] = gender;

                    std::string english_word;
                    out.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the first line
                    for (std::string line; std::getline(out, line, '\n');) {
                        pw::string::trim_right(line);
                        if (line.back() == ']') {
                            for (char c; out.get(c) && !ispunct(c);) {
                                english_word.push_back(c);
                            }
                            pw::string::trim_right(english_word);
                            break;
                        }
                    }

                    // Decline English word
                    // if (plurality[0] == 'P') {
                    //     switch (english_word.back()) {
                    //     case 'a':
                    //     case 'e':
                    //     case 'i':
                    //     case 'o':
                    //     case 'u':
                    //         english_word.push_back('e');
                    //         break;
                    //     }
                    //     english_word.push_back('s');
                    // }
                    resp["english_word"] = english_word;

                    break;
                }
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
