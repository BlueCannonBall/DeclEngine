import sys
import requests

if len(sys.argv) < 3:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

input_dataset = open(sys.argv[1], "rt", buffering=1)
output_dataset = open(sys.argv[2], "wt", buffering=1)

i = 0
while True:
    line = input_dataset.readline()
    if line:
        split_line = line.strip().split('\t')
        latin = split_line[0]
        r = requests.get(
            "http://localhost:8000/sentence_info", params={"sentence": latin})
        if r.status_code != 200:
            print(
                f"Warning: The following sentence could not be converted to IR: {latin}", file=sys.stderr)
            continue
        ir = r.text
        english = split_line[1]
        output_dataset.write(f"{ir}\t{english}\n")

        i += 1
        if i % 100 == 0:
            print(f"Done with {i} sentence pairs")
    else:
        if i % 100 != 0:
            print(f"Done with {i}/{i} sentence pairs")
        break

input_dataset.close()
output_dataset.close()
