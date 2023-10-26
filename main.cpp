#include "Polyweb/polyweb.hpp"
#include "json.hpp"
#include <boost/process.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <cctype>

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
                        pw::string::trim(line);
                        if (line.back() == ';') {
                            for (char c : line) {
                                if (ispunct(c) || isspace(c)) {
                                    break;
                                } else {
                                    english_word.push_back(c);
                                }
                            }
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
                    int conjugation;
                    std::string tense, voice, mood, plurality;
                    out >> conjugation >> unknown >> tense >> voice >> mood >> unknown >> plurality;
                    resp["conjugation"] = conjugation;
                    resp["tense"] = tense;
                    resp["voice"] = voice;
                    resp["mood"] = mood;
                    resp["plural"] = plurality[0] == 'P';
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
