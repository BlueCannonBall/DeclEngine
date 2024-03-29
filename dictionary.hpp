#pragma once

#include "words.hpp"
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
    std::string breakdown;

    static WordVariant make_proper_noun(const std::string& english_base) {
        return {
            .forms = {
                std::make_shared<Noun>(0, CASUS_NOMINATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_GENITIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_DATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_ACCUSATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_ABLATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_VOCATIVE, false, GENDER_COMMON),
                std::make_shared<Noun>(0, CASUS_LOCATIVE, false, GENDER_COMMON),
            },
            .english_base = english_base,
        };
    }

    bool is_valid() const {
        return !forms.empty();
    }

    operator bool() const {
        return is_valid();
    }
};

class Transliterator {
protected:
    locale_t us_locale;
    iconv_t cd;

public:
    Transliterator():
        us_locale(newlocale(LC_CTYPE_MASK, "en_US.utf8", nullptr)),
        cd(iconv_open("ASCII//TRANSLIT", "UTF-8")) {}

    ~Transliterator() {
        iconv_close(cd);
        freelocale(us_locale);
    }

    std::string operator()(const std::string& str);
};

size_t query_dictionary(const std::string& word, std::vector<WordVariant>& ret);
