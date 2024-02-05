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

$ curl "http://localhost:8000/sentence_info?sentence=In+principio+creavit+Deus+caelum+et+terram."
<F:PREP><C:ABL>in<S><F:N><C:ABL><P:F><G:N>beginning<S><F:V><T:PERF><V:A><M:IND><PPL:3><P:F>create<S><F:N><C:N><P:F><G:M>God<S><F:N><C:ACC><P:F><G:M>heaven<S><F:C>and<S><F:N><C:ACC><P:F><G:F>earth.
```
