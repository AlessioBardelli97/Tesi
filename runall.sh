#!/bin/bash

target=(generalized_test_Filippo_migliorato)

for i in pla/*; do
	for j in ${target[*]}; do
		./$j.out $i > output.${i#pla/}.$j.txt
	done
done
