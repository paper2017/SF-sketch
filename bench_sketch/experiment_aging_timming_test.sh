#!/bin/bash
#sh experiment_zipf_inc.sh 5 40000 3 24 100 100 1 
#exit
ROUNDS=10
for ((i=1; i<=$ROUNDS; i++));do
	echo ------------ round $i ----------------
	sh experiment_zipf_inc.sh 5 40000 3 24 100 100 1
done
