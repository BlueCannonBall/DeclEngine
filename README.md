# DeclEngine
Providing powerful symbolic analysis of Latin words and sentences backed by rock-solid computational intelligence. ✨

## Usage
See `test.py` for a simple way to test and query the engine in the command line. In the example below, the engine is queried manually using the `curl` command. Make sure to execute the engine in the project's root directory.
```sh
$ curl "http://localhost:8000/word_info?word=amat"
[
    {
        "english_base": "love",
        "forms": [
            {
                "english_equivalent": "it loves",
                "mood": "indicative",
                "part_of_speech": "verb",
                "plural": false,
                "tense": "present",
                "voice": "active"
            }
        ]
    }
]

$ curl "http://localhost:8000/sentence_info?sentence=In%20principio%20creavit%20Deus%20caelum%20et%20terram."
in<SEP><ABL>beginning,<SEP>it created<SEP><NOM>God<SEP><ACC>heaven<SEP>and<SEP><ACC>earth.
```
