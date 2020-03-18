#!/bin/bash
LONG_BIT=$(getconf LONG_BIT)
D=$1
W=$2
K=$3
BITS_C=$4
START=$5
END=$6
DELTA=$7
STREAMID=$8

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

COLOR(){
    echo "\033[34m${1}\033[0m"
}

RED(){
    echo "\033[31m${1}\033[0m"
}

CHSETTING(){
    ((OP_K=$1 * $BASE_OP_K))
    ((OP=$1 * $BASE_OP))
#    SETTING=kv${BASE_ITEMS_K}K_op${OP_K}K_uniform_$2_${D}_${W}_${K}_${BITS_C}
    SETTING=stream${STREAMID}_$2
}


CLEAN(){
#    for ((i=$START;i<=$END;i+=$DELTA));do
        CHSETTING $START real

#        rm $REQ_DIR/${SETTING}_init
        rm $REQ_DIR/${SETTING}_inc
        rm $REQ_DIR/${SETTING}_query

    for ((i=$START-$DELTA;i>=$END;i-=$DELTA));do
        CHSETTING $DELTA real_inc$i
        rm $REQ_DIR/${SETTING}
#        rm $LOG_DIR/${SETTING}_inc-dec_up.dat
    done
#        rm $LOG_DIR/${SETTING}_init.dat
#        rm $LOG_DIR/${SETTING}_inc-dec_up.dat

#        rm ${SETTING_DIR}/${SETTING}.dat
#    done
}
#################################

if     [ "$#" -ne 8 ] || [ "$W" -lt 1 ] || [ "$D" -lt 1 ]\
    || [ "$K" -lt 1 ] || [ "${BITS_C}" -lt 1 ] || [ "${START}" -lt 1 ]\
    || [ "${END}" -lt "${START}" ] || [ "${DELTA}" -lt 1 ]
then
    echo -e We consider `RED 100K` items with frequents `RED "REAL distributed"`
    echo -e while in each experiment keep different frequent `RED INCREASED` by `COLOR DELTA``RED '(must > 0)'` from `COLOR START` to `COLOR END`
    echo -e on-chip, we use `COLOR W` buckets in each of the `COLOR D` tables, while using `COLOR BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR K*W` buckets in each of the `COLOR D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
    echo -e usage: ./experiment_real_inc.sh `COLOR D` `COLOR W` `COLOR K` `COLOR BITS_C` `COLOR START` `COLOR END` `COLOR DELTA` `COLOR STREAMID`
    exit
else
    echo -e With Real Stream `COLOR $STREAMID` We experiment `COLOR 100K` items with frequents `RED "REAL distributed"`
    echo -e while in each experiment keeping different frequent `RED INCREASED` by `COLOR $DELTA` from `COLOR $START` to `COLOR $END`
    echo -e on-chip, we use `COLOR $W` buckets in each of the `COLOR $D` tables, while using `COLOR $BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR $K*$W` buckets in each of the `COLOR $D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
fi

EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${START}_${END}_${DELTA}_real_inc_${D}_${W}_${K}_${BITS_C}

for ((i=$START;i<=$END;i+=$DELTA))
do
    CHSETTING $i real
#    SETTING_FILE=${SETTING_DIR}/${SETTING}.dat
#    echo $SETTING_FILE
#    if [ -f $SETTING_FILE ]
#    then
#        rm $SETTING_FILE
#    fi
#    cp $TEMPLET $SETTING_FILE
#    $(sed -i "9s/XXXK/${OP_K}K/" $SETTING_FILE)
#    $(sed -i "10s/XXXK/$OP/" $SETTING_FILE)
#    sh ./ycsb_workload_gen.sh $SETTING_FILE
#    sh ./benchmark_creat.sh ${SETTING} $EXP_DIR
PIDFILE=${CUR_DIR}/pid.tmp

INC_DEC_UP=inc
QUERY=query

#    echo usage: sh benchmark_creat.sh [setting] [EXP_DIR] [reverse] [delta] [# of delta]

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

if [ ! -f ${REQ_DIR}/${SETTING}_${INC_DEC_UP} ]
then
    echo We need Real Query Stream: ${REQ_DIR}/${SETTING}_${INC_DEC_UP}
    exit 0
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


#    sh ./other_sketch_query_creat.sh ${SETTING} $EXP_DIR $D $W $K $BITS_C
done

#CLEAN
