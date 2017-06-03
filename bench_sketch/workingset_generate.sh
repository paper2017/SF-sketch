#!/bin/bash
CUR_DIR=$(pwd)
EXP_DIR=${CUR_DIR}/to_github

sh ./ycsb_workload_gen.sh ./templets/kv1k_op10K.dat
sh ./benchmark_creat.sh kv1k_op10K $EXP_DIR

cp ./requests/kv1k_op10K* $EXP_DIR

#sh ./ycsb_workload_gen.sh ./templets/kv1k_op500K_uniform.dat
#sh ./benchmark_creat.sh kv1k_op500K_uniform $EXP_DIR

#sh ./ycsb_workload_gen.sh ./templets/kv10k_op1000K_uniform.dat
#sh ./benchmark_creat.sh kv10k_op1000K_uniform $EXP_DIR



#sh ./ycsb_workload_gen.sh ./templets/kv100k_op5000K_uniform.dat
#sh ./benchmark_creat.sh kv100k_op5000K_uniform $EXP_DIR
