#include "Polyweb/polyweb.hpp"
#include "json.hpp"
#include "translate.hpp"
#include <iostream>
#include <string>

using nlohmann::json;

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

                WordInfo word_info = query_whitakers_words(word_it->second);
                json resp = {
                    {"split_word", word_info.split_word},
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
                        case TENSE_FUTURE_PERFECT: json_variant["tense"] = "future perfect"; break;
                        }

                        switch (verb->voice) {
                        case VOICE_ACTIVE: json_variant["voice"] = "active"; break;
                        case VOICE_PASSIVE: json_variant["voice"] = "passive"; break;
                        }

                        switch (verb->mood) {
                        case MOOD_INDICATIVE: json_variant["mood"] = "indicative"; break;
                        case MOOD_INFINITIVE: json_variant["mood"] = "infinitive"; break;
                        case MOOD_IMPERATIVE: json_variant["mood"] = "imperative"; break;
                        case MOOD_SUBJUNCTIVE: json_variant["mood"] = "subjunctive"; break;
                        }

                        json_variant["plural"] = verb->plural;

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
                        }

                        break;
                    }
                    }

                    json_variant["english_equivalent"] = variant->english_equivalent(word_info.english_base);
                    resp["variants"].push_back(json_variant);
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
