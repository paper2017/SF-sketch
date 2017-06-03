#!/bin/bash

for ((round=1;round<=10;round++))
do
	echo -----------Round ${round} ------------
	for ((i=1;i<=10;i++))
	do
	   ((w= ${i} * 10000))
	   echo sh experiment_uniform_inc.sh 4 ${w} 4 32 100 100 1 AGING
	   sh experiment_uniform_inc.sh 4 ${w} 4 32 100 100 1 AGING
	done
done
