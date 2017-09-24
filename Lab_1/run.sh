#!/usr/bin/env bash
for ((count=1000; count <= 2000; count += 1))
do
./cache $count
done

