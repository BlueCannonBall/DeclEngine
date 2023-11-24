import sys
import requests

if len(sys.argv) < 3:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

input_dataset = open(sys.argv[1], 'r')
output_dataset = open(sys.argv[2], 'w')

i = 0
while True:
    line = input_dataset.readline()
    if line:
        split_line = line.strip().split('\t')
        latin = split_line[0]
        dirty_english = requests.get(
            "http://localhost:8000/sentence_info", params={"sentence": latin}).content.decode()
        clean_english = split_line[1]
        output_dataset.write(f"{dirty_english}\t{clean_english}\n")

        i += 1
        if i % 25 == 0:
            print(f"Done with {i} sentence pairs")
    else:
        if i % 25 != 0:
            print(f"Done with {i}/{i} sentence pairs")
        break

input_dataset.close()
output_dataset.close()
