#pragma once

#include "words.hpp"
#include <boost/process.hpp>
#include <clocale>
#include <cstddef>
#include <iconv.h>
#include <memory>
#include <string>
#include <vector>

constexpr size_t hash(const char* str, size_t i = 0) {
    return !str[i] ? 5381 : (hash(str, i + 1) * 33) ^ str[i];
}

inline size_t hash(const std::string& str, size_t i = 0) {
    return !str[i] ? 5381 : (hash(str, i + 1) * 33) ^ str[i];
}

struct WordVariant {
    std::vector<std::shared_ptr<WordForm>> forms;
    std::string english_base;

    bool is_valid() const {
        return !forms.empty();
    }

    PartOfSpeech part_of_speech() const {
        return forms.front()->part_of_speech;
    }
};

class WhitakersWords {
private:
    locale_t us_locale = newlocale(LC_CTYPE_MASK, "en_US.utf8", nullptr);
    iconv_t cd = iconv_open("ASCII//TRANSLIT", "UTF-8");

public:
    boost::process::ipstream out;
    boost::process::opstream in;
    boost::process::child child;

    WhitakersWords(const std::string& binary = "bin/words", const std::string& start_dir = "whitakers-words"):
        child(binary, boost::process::start_dir(start_dir), boost::process::std_out > out, boost::process::std_in < in) {}

    ~WhitakersWords() {
        iconv_close(cd);
        freelocale(us_locale);
    }

    std::string remove_accents(const std::string& str);
};

size_t query_dictionary(const std::string& word, std::vector<WordVariant>& ret);
