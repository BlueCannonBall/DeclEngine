import sys

if len(sys.argv) < 4:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

input_dataset = open(sys.argv[1], "rt", buffering=1)
left_output_dataset = open(sys.argv[2], "wt", buffering=1)
right_output_dataset = open(sys.argv[3], "wt", buffering=1)

while True:
    line = input_dataset.readline()
    if line:
        split_line = line.strip().split('\t')
        left_output_dataset.write(split_line[0] + '\n')
        right_output_dataset.write(split_line[1] + '\n')
    else:
        break

input_dataset.close()
left_output_dataset.close()
left_output_dataset.close()
