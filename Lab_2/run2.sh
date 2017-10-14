#!/usr/bin/env bash

for n in {500..1000..10}
do
    for m in {10..30}
    do
        ./main $n $m
        echo "Done n=$n m=$m"
    done
done
