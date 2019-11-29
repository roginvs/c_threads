#!/bin/bash
set -e

echo "==== Running unit tests ===="
gcc -Wall -o test.out test.c
./test.out

echo ""
echo ""


echo "Building executable"

gcc -Wall -o main.out main.c -lpthread


DIR=`mktemp -d`

for fFull in `ls test/*`; do
  f=`basename "${fFull}"`
  echo "==== Testing $f file ===="
  ./main.out "test/$f" "$DIR/$f.gz"
  cat "$DIR/$f.gz" | gzip -d > "$DIR/$f.raw"
  cmp "test/$f" "$DIR/$f.raw"
  echo ""
done

echo "All done"
