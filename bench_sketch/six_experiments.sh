#!/bin/bash
echo --------------------------UNIFORM--------------------------
echo --------------sh experiment_uniform_inc.sh 5 40000 3 24 5 100 5--------------
sh experiment_uniform_inc.sh 5 40000 3 24 5 100 5 GEN

echo --------------sh experiment_uniform_dec.sh 5 40000 3 24 100 0 5--------------
sh experiment_uniform_dec.sh 5 40000 3 24 100 0 5 GEN

echo --------------sh experiment_uniform_inc-dec.sh 5 40000 3 24 100 5--------------
sh experiment_uniform_inc-dec.sh 5 40000 3 24 100 5 GEN

echo --------------------------ZIPFIAN--------------------------
echo --------------sh experiment_zipf_inc.sh 5 40000 3 24 5 100 5--------------
sh experiment_zipf_inc.sh 5 40000 3 24 5 100 5 GEN

echo --------------sh experiment_zipf_dec.sh 5 40000 3 24 100 0 5--------------
sh experiment_zipf_dec.sh 5 40000 3 24 100 0 5 GEN

echo --------------sh experiment_zipf_inc-dec.sh 5 40000 3 24 100 5--------------
sh experiment_zipf_inc-dec.sh 5 40000 3 24 100 5 GEN