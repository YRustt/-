#!/usr/bin/env bash

for n in {4000..6000..100}
do
    for m in {10..20}
    do
        ./main $n $m
        echo "Done n=$n m=$m"
    done
done
