import sys
import requests
import json

if len(sys.argv) < 2:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

if ' ' in sys.argv[1]:
    print(requests.get("http://localhost:8000/sentence_info",
                       params={"sentence": sys.argv[1]}).content.decode())
else:
    r = requests.get("http://localhost:8000/word_info",
                     params={"word": sys.argv[1]})
    if r.status_code != 200:
        print(r.text)
    else:
        print(json.dumps(r.json(), indent=4))
