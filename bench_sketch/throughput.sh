#!/bin/bash
#sh ycsb_workload_gen.sh settings/op500K_incdec.dat
#mkdir throughputs_test
CUR_DIR=$(pwd)
EXP_DIR=${CUR_DIR}/throughputs_test
#sh ./benchmark_creat.sh op500K_incdec $EXP_DIR
sh ./other_sketch_query_creat.sh op500K_thrpt $EXP_DIR 5 40000 3 24 throughput
