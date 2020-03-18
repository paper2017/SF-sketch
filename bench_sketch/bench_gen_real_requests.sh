#!/bin/bash
LONG_BIT=$(getconf LONG_BIT)
STREAMID=$1

#sh experiment_real_inc_get_distitems.sh 5 40000 3 24 1 100 1 004

#we create 100K items
BASE_ITEMS_K=100
BASE_OP_K=100
BASE_OP=100000
#TEMPLET=./templets/kv100K_opXXXK_uniform.dat
#SETTING_DIR=./settings
#LOG_DIR=./workingsets
REQ_DIR=./real_stream
CUR_DIR=$(pwd)
EXP_DIR=${CUR_DIR}/raw_data_experiment
#BLUE="\033[34m$BLUE\033[0m"


INC_DEC_UP=inc
QUERY=query

COLOR(){
    echo "\033[34m${1}\033[0m"
}

RED(){
    echo "\033[31m${1}\033[0m"
}
\
SETTING=stream${STREAMID}_real

CLEAN(){
        CHSETTING
        rm $REQ_DIR/${SETTING}_${INC_DEC_UP}
        rm $REQ_DIR/${SETTING}_${QUERY}

}
#################################

if     [ "$#" -ne 1 ]
then
    echo -e usage: ./bench_gen_real_requests.sh `COLOR STREAMID`
    exit
fi

EXP_DIR=${CUR_DIR}/raw_data_experiment/BENCH_real_inc_of_${STREAMID}


PIDFILE=${CUR_DIR}/pid.tmp


#    echo usage: sh benchmark_creat.sh [setting] [EXP_DIR] [reverse] [delta] [# of delta]

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

if [ ! -f ${REQ_DIR}/${SETTING}_${INC_DEC_UP} ]
then
    echo We need Real Query Stream: ${REQ_DIR}/${SETTING}_${INC_DEC_UP}
    exit 0
fi

if [ -f $PIDFILE ]; then
    PID=$(cat $PIDFILE)
    kill $PID
rm $PIDFILE
fi

echo use workingset $SETTING to create benchmark

echo open memcached service

memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1

echo create benchmark

if [ -f ${REQ_DIR}/${SETTING}_${QUERY} ];then
    rm ${REQ_DIR}/${SETTING}_${QUERY}
fi
./bench_client4Real -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING}_${INC_DEC_UP} -o ${REQ_DIR}
./bench_client -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING}_${QUERY}

#cp ./benchmarks/${setting}.bench $EXP_DIR/${setting}.bench

    mv ./benchmarks/${SETTING}.bench $EXP_DIR/${SETTING}.bench

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE

echo memcached service closed


#CLEAN
