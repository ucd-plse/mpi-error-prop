#!/bin/sh

for ((  i = 1 ;  i <= $1;  i++  )) do
  for trans in transfer copy; do
    echo "Replacing test$i/$trans.expected with test$i/$trans.out"
    mv "test$i/$trans.out" "test$i/$trans.expected"
  done
done
