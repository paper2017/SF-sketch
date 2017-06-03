#!/bin/bash

for ((round=1;round<=10;round++))
do
	echo -----------Round ${round} ------------
	for ((i=1;i<=1;i++))
	do
	   ((w= 40000))
	   echo sh experiment_uniform_inc.sh 4 ${w} 4 32 100 100 1 TIME
	   sh experiment_uniform_inc.sh 4 ${w} 4 24 100 100 1 TIME
	done
done
