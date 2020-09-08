#!/bin/bash

target=(all_zeros_test all_ones_test generalized_test_Filippo generalized_test_Alessio generalized_test_Filippo_migliorato generalized_test_Ottimo)

for i in ${target[*]}; do
	./$i.out $1 > output.$i.txt
done

# python3 extract.py

