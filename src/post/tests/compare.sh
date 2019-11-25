#!/bin/sh

for ((  i = 1 ;  i <= $1;  i++  )) do
  for trans in transfer copy; do
    echo "Comparing test$i/$trans.expected and test$i/$trans.out"
    diff "test$i/$trans.expected" "test$i/$trans.out"
  done
done
