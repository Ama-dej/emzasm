#!/bin/bash

cd test

for i in *
do
	y=${i%.*}
	echo $i
	./../bin/emzasm $i ../bin/$y.bin
done
