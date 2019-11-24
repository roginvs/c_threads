#!/bin/bash
set -e

echo "==== Running unit tests ===="
gcc -o test.out test.c
./test.out

echo ""
echo ""


echo "Building executable"

gcc -o main.out main.c -lpthread


for fFull in `ls test/*`; do
  f=`basename "${fFull}"`
  echo "==== Testing $f file ===="
  ./main.out "test/$f" "/tmp/out-$f"
  cat "/tmp/out-$f" | gzip -d > "/tmp/out-uncompressed-$f"
  cmp "test/$f" "/tmp/out-uncompressed-$f"
  echo ""
done

echo "All done"
