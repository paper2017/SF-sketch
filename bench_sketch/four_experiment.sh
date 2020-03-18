#!/bin/bash
echo sh experiment_uniform_inc.sh 5 40000 3 24 5 15 5
sh experiment_uniform_inc.sh 5 40000 3 24 5 15 5

echo sh experiment_uniform_dec.sh 5 40000 3 24 15 0 5
sh experiment_uniform_dec.sh 5 40000 3 24 15 0 5

echo --------------sh experiment_uniform_inc-dec.sh 5 40000 3 24 15 5--------------
sh experiment_uniform_inc-dec.sh 5 40000 3 24 15 5


echo sh experiment_zipf_inc.sh 100 5 40000 3 24 5 15 5
sh experiment_zipf_inc.sh 100 5 40000 3 24 5 15 5

echo sh experiment_zipf_dec.sh 5 40000 3 24 15 0 5
sh experiment_zipf_dec.sh 5 40000 3 24 15 0 5



echo --------------sh experiment_zipf_inc-dec.sh 5 40000 3 24 15 5--------------
sh experiment_zipf_inc-dec.sh 5 40000 3 24 15 5
