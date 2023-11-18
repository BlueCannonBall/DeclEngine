#pragma once

#include "words.hpp"
#include <cstddef>
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

std::string remove_accents(const std::string& str);
size_t query_dictionary(const std::string& word, std::vector<WordVariant>& ret);
