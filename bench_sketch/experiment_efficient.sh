#!/bin/bash
echo ---------fix D=5------------
sh experiment_uniform_inc.sh 5 10000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 20000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 30000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 40000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 50000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 60000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 70000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 80000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 90000 3 24 100 100 1
sh experiment_uniform_inc.sh 5 100000 3 24 100 100 1

echo ---------fix W=40k------------
sh experiment_uniform_inc.sh 1 40000 3 24 100 100 1
sh experiment_uniform_inc.sh 2 40000 3 24 100 100 1
sh experiment_uniform_inc.sh 3 40000 3 24 100 100 1
sh experiment_uniform_inc.sh 4 40000 3 24 100 100 1
sh experiment_uniform_inc.sh 6 40000 3 24 100 100 1

echo ---------fix D * W=200k------------
sh experiment_uniform_inc.sh 1 200000 3 24 100 100 1
sh experiment_uniform_inc.sh 2 100000 3 24 100 100 1
sh experiment_uniform_inc.sh 3 66666 3 24 100 100 1
sh experiment_uniform_inc.sh 4 50000 3 24 100 100 1
sh experiment_uniform_inc.sh 6 33333 3 24 100 100 1
sh experiment_uniform_inc.sh 7 28571 3 24 100 100 1