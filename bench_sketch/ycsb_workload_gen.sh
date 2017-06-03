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
    echo usage: ./ycsb_workload_gen.sh [your_setting_file]
    exit 0
fi

setting=${setting_raw%.*}
setting=${setting##*/}

echo using setting file $setting to create workingsets
echo using predefined workloadc to create transaction records for $setting with reads only

echo generating ${setting}_${INIT}, the init update used before benchmark
eval ${YCSB_HOME}/bin/ycsb load ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $setting_raw > ${LOG_DIR}/${setting}_${INIT}.dat

echo generating ${setting}_${INC_DEC_UP}, the increase/decrease update used before benchmark
eval ${YCSB_HOME}/bin/ycsb run ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $setting_raw > ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat

echo generating requests/operations

CUR_DIR=$(pwd)
PIDFILE=${CUR_DIR}/pid.tmp
CHECK_FILE=uniform_check
echo open memcached service

if [ "$OPT_ARG"A = uniformA ];then
    rm $CHECK_FILE
fi
rm ${REQ_DIR}/${setting}_${INC_DEC_UP}

/opt/memcached-master/bin/memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1
echo generate query operations
./bench_query_gen ${REQ_DIR}/${setting}_${QUERY} < ${LOG_DIR}/${setting}_${INIT}.dat
echo generate init operations
./bench_update_gen -o ${REQ_DIR}/${setting}_${INIT} -i ${LOG_DIR}/${setting}_${INIT}.dat -l

echo generate inc/dec operations
./bench_update_gen -o ${REQ_DIR}/${setting}_${INC_DEC_UP} -i ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat


if [ "$OPT_ARG"A = uniformA ];then
    echo check uniform frequent
    for((i=0;i<50;i++));do
        isOK=$(sed -n '1p' $CHECK_FILE )
        if [ $isOK = "true" ];then
            echo CHECK UNIFORM FREQUENT OK
            break;
        else
            echo generate operations for uniform frequent : $i
            counter=$(sed -n '2p' $CHECK_FILE )
            echo $counter items have satisfy uniform-frequent requirement
            eval ${YCSB_HOME}/bin/ycsb run ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $setting_raw $p1 $p2 $p3 $status > ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat
            ./bench_update_gen -o ${REQ_DIR}/${setting}_${INC_DEC_UP} -i ${LOG_DIR}/${setting}_${INC_DEC_UP}.dat -c ${REQ_DIR}/${setting}_${QUERY} -a
        fi
    done
fi

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE
echo memcached service closed

