import sys

if len(sys.argv) < 4:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

left_dataset = open(sys.argv[1], "rt", buffering=1)
right_dataset = open(sys.argv[2], "rt", buffering=1)
output_dataset = open(sys.argv[3], "wt", buffering=1)

while True:
    left_line = left_dataset.readline()
    right_line = right_dataset.readline()
    if left_line and right_line:
        output_dataset.write(left_line.rstrip() + '\t' + right_line)
    else:
        break

left_dataset.close()
right_dataset.close()
output_dataset.close()
