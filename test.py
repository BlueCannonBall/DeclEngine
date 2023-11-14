import sys
import requests

if len(sys.argv) < 3:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

match sys.argv[1]:
    case "word":
        print(requests.get("http://localhost:8000/word_info", params={"word": sys.argv[2]}).content.decode())
    case "sentence":
        print(requests.get("http://localhost:8000/sentence_info", params={"sentence": sys.argv[2]}).content.decode())
    case _:
        print("Invalid operation", file=sys.stderr)
