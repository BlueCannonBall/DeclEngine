import requests
from itertools import groupby

input_text = "nos sumus milites audaces".split(' ')
output_text = []

words = []
offset = 0
for word in input_text:
    words.append(min(requests.get("http://localhost:8000/word_info", params={"word": word}).json(), key=lambda a: len(a["english_base"])))

# Cull impossible variants
for i in range(offset, len(words) - 1):
    for preposition in words[i]["variants"]:
        if preposition["part_of_speech"] == "preposition":
            for noun in words[i + 1]["variants"]:
                if noun["part_of_speech"].endswith("noun") and noun["casus"] == preposition["casus"]:
                    words[i + 1]["variants"] = [noun]

# Find subject
for i in range(offset, len(words)):
    subject_found = False
    for noun in words[i]["variants"]:
        if noun["part_of_speech"].endswith("noun") and noun["casus"] == "nominative":
            adjectives = []
            for j in range(i + 1, len(words)):
                adjective_found = False
                for adjective in words[j]["variants"]:
                    if adjective["part_of_speech"] == "adjective":
                        adjectives.append(adjective)
                        adjective_found = True
                        break
                if not adjective_found:
                    break
            for adjective in adjectives:
                output_text.append(adjective["english_equivalent"])
                offset += 1
            output_text.append(noun["english_equivalent"])
            offset += 1
            subject_found = True
            break
    if subject_found:
        break

# Find verb
for i in range(offset, len(words)):
    verb_found = False
    for verb in words[i]["variants"]:
        if verb["part_of_speech"] == "verb":
            output_text.append(verb["english_equivalent"])
            offset += 1
            verb_found = True
            break
    if verb_found:
        break


# Find object
for i in range(offset, len(words)):
    object_found = False
    for noun in words[i]["variants"]:
        if noun["part_of_speech"].endswith("noun") and noun["casus"] == "accusative":
            adjectives = []
            for j in range(i + 1, len(words)):
                adjective_found = False
                for adjective in words[j]["variants"]:
                    if adjective["part_of_speech"] == "adjective":
                        adjectives.append(adjective)
                        adjective_found = True
                        break
                if not adjective_found:
                    break
            for adjective in adjectives:
                output_text.append(adjective["english_equivalent"])
                offset += 1
            output_text.append(noun["english_equivalent"])
            offset += 1
            object_found = True
    if object_found:
        break



print("LATIN:", ' '.join(input_text))
print("ENGLISH:", ' '.join([key for key, _group in groupby(' '.join(output_text).split(' '))]))

# for word in words:
#     print(word["variants"][0]["english_equivalent"]):

