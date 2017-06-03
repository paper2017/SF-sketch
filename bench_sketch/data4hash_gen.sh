#!/bin/bash
YCSB_HOME=/opt/ycsb-0.5.0-SNAPSHOT
LOG_DIR=./workingsets
REQ_DIR=./requests
DATABASE=basic
setting=
p1=
p2=
p3=
status=


INIT=init
INIT_UP=init_up
INC_DEC_UP=inc-dec_up
QUERY=query
setting_raw=$1
OPT_ARG=$2

if [ "$OPT_ARG"A = uniformA ];then
    echo $OPT_ARG
fi

if [ -z $setting_raw ] || [ ! -f $setting_raw ];then
    echo usage: ./data4hash_gen.sh [your_setting_file] [a|b|c]
    exit 0
fi

setting=${setting_raw%.*}
setting=${setting##*/}

echo using setting file $setting to create workingsets
echo using predefined workloadc to create transaction records for $setting with reads only

echo generating ${setting}_${INIT}, the init update used before benchmark
eval ${YCSB_HOME}/bin/ycsb load ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $setting_raw > ${LOG_DIR}/${setting}_${INIT}.dat

echo generating ${setting}_${INC_DEC_UP}, the increase/decrease update used before benchmark
#eval ${YCSB_HOME}/bin/ycsb run ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $setting_raw > ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat

echo generating requests/operations

CUR_DIR=$(pwd)
PIDFILE=${CUR_DIR}/pid.tmp
CHECK_FILE=uniform_check

#rm ${REQ_DIR}/${setting}_${INC_DEC_UP}


echo generate query operations $OPT_ARG
./data4hash $OPT_ARG ${REQ_DIR}/RAWDATA_${OPT_ARG} < ${LOG_DIR}/${setting}_${INIT}.dat

echo generate init operations
#./bench_update_gen -o ${REQ_DIR}/${setting}_${INIT} -i ${LOG_DIR}/${setting}_${INIT}.dat -l

echo generate inc/dec operations
#./bench_update_gen -o ${REQ_DIR}/${setting}_${INC_DEC_UP} -i ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat


