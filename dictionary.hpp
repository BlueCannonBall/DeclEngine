#pragma once

#include "words.hpp"
#include <iconv.h>
#include <locale.h>
#include <memory>
#include <stddef.h>
#include <string>
#include <string_view>
#include <vector>

constexpr size_t hash(std::string_view str, size_t i = 0) {
    return i == str.size() ? 5381 : (hash(str, i + 1) * 33) ^ str[i];
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

    std::string operator()(std::string_view str);
};

size_t query_dictionary(const std::string& word, std::vector<WordVariant>& ret);
