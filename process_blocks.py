import sys

if len(sys.argv) < 2:
    print("Error: Too few arguments", file=sys.stderr)
    sys.exit(1)

letters = open(sys.argv[1], "rt", buffering=1)

i = 0
while True:
    line = letters.readline()
    if line:
        i += 1
        split_line = line.split('\t')
        print(i, split_line[0].count('.') + split_line[0].count('?') + split_line[0].count('!') + split_line[0].count(
            ';'), split_line[1].count('.') + split_line[1].count('?') + split_line[1].count('!') + split_line[1].count(';'))
    else:
        break

letters.close()
