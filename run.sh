#!/usr/bin/bash

rm -f -r output_Alessio
rm -f -r output_Filippo

mkdir output_Alessio
mkdir output_Filippo

cd pla/

for filename in *.pla
	do
		../generalized_test_Alessio.out $filename > ../output_Alessio/$filename.txt
		../generalized_test_Filippo.out $filename > ../output_Filippo/$filename.txt
	done
