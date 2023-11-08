import requests
from itertools import groupby

input_text = "tu stultum es".split(' ')
output_text = []

words = []
for word in input_text:
    words.append(min(requests.get("http://localhost:8000/word_info", params={"word": word}).json(), key=lambda a: len(a["english_base"])))

# Cull impossible variants
for i in range(len(words) - 1):
    for preposition in words[i]["variants"]:
        if preposition["part_of_speech"] == "preposition":
            for noun in words[i + 1]["variants"]:
                if noun["part_of_speech"].endswith("noun") and noun["casus"] == preposition["casus"]:
                    words[i + 1]["variants"] = [noun]

remove = []

# Find subject
for i in range(len(words)):
    subject_found = False
    for noun in words[i]["variants"]:
        if noun["part_of_speech"].endswith("noun") and noun["casus"] == "nominative":
            remove.append(words[i])
            adjectives = []
            for j in range(i + 1, len(words)):
                adjective_found = False
                for adjective in words[j]["variants"]:
                    if adjective["part_of_speech"] == "adjective":
                        remove.append(words[j])
                        adjectives.append(adjective)
                        adjective_found = True
                        break
                if not adjective_found:
                    break
            for adjective in adjectives:
                output_text.append(adjective["english_equivalent"])
            output_text.append(noun["english_equivalent"])
            subject_found = True
            break
    if subject_found:
        break

for word in remove:
    words.remove(word)
remove.clear()

# Find verb
for i in range(len(words)):
    verb_found = False
    for verb in words[i]["variants"]:
        if verb["part_of_speech"] == "verb":
            remove.append(words[i])
            output_text.append(verb["english_equivalent"])
            verb_found = True
            break
    if verb_found:
        break

for word in remove:
    words.remove(word)
remove.clear()

# Find object
for i in range(len(words)):
    object_found = False
    for noun in words[i]["variants"]:
        if noun["part_of_speech"].endswith("noun") and noun["casus"] == "accusative":
            remove.append(words[i])
            adjectives = []
            for j in range(i + 1, len(words)):
                adjective_found = False
                for adjective in words[j]["variants"]:
                    if adjective["part_of_speech"] == "adjective":
                        remove.append(words[j])
                        adjectives.append(adjective)
                        adjective_found = True
                        break
                if not adjective_found:
                    break
            for adjective in adjectives:
                output_text.append(adjective["english_equivalent"])
            output_text.append(noun["english_equivalent"])
            object_found = True
    if object_found:
        break

for word in remove:
    words.remove(word)
remove.clear()

print("LATIN:", ' '.join(input_text))
print("ENGLISH:", ' '.join([key for key, _group in groupby(' '.join(output_text).split(' '))]))
