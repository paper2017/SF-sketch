#!/bin/bash
LONG_BIT=$(getconf LONG_BIT)
D=$1
W=$2
K=$3
BITS_C=$4
START=0
END=$5
DELTA=$6

#sh experiment_uniform_inc-dec.sh 5 40000 3 24 1 100 1

#we create 100K items
BASE_ITEMS_K=100
BASE_OP_K=100
BASE_OP=100000
TEMPLET=./templets/kv100K_opXXXK_uniform.dat
SETTING_DIR=./settings
LOG_DIR=./workingsets
REQ_DIR=./requests
CUR_DIR=$(pwd)
EXP_DIR=${CUR_DIR}/raw_data_experiment
SETTING_SUF=incdec
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
    SETTING=op${OP_K}K_$2
}


CLEAN(){
    CHSETTING $END $SETTING_SUF

    rm $REQ_DIR/${SETTING}_init
    rm $REQ_DIR/${SETTING}_inc-dec_up
    rm $REQ_DIR/${SETTING}_query

    rm $LOG_DIR/${SETTING}_init.dat
    rm $LOG_DIR/${SETTING}_inc-dec_up.dat
}
#################################

if     [ "$#" -ne 6 ] || [ "$W" -lt 1 ] || [ "$D" -lt 1 ]\
    || [ "$K" -lt 1 ] || [ "${BITS_C}" -lt 1 ] \
    || [ "${END}" -lt 1 ] || [ "${DELTA}" -lt 1 ]
then
    echo -e We consider `RED 100K` items with operation frequents obeying `RED "uniform distribution"`
    echo -e record the frequency of each item in every `COLOR DELTA*100K` operations.
    echo -e apply `COLOR END*100K` insert operations and then delete operations in a reverse order.
    echo -e on-chip, we use `COLOR W` buckets in each of the `COLOR D` tables, while using `COLOR BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR K*W` buckets in each of the `COLOR D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
    echo -e usage: ./experiment_uniform_inc-dec.sh `COLOR D` `COLOR W` `COLOR K` `COLOR BITS_C`  `COLOR END` `COLOR DELTA`
    exit
else
    echo -e We consider `RED 100K` items with operation frequents obeying `RED "uniform distribution"`
    echo -e record the frequency of each item in every `COLOR ${DELTA}*100K` operations.
    echo -e apply `COLOR ${END}*100K` insert operations and then delete operations in a reverse order.
    echo -e on-chip, we use `COLOR $W` buckets in each of the `COLOR $D` tables, while using `COLOR $BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR $K*$W` buckets in each of the `COLOR $D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent

fi

EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${END}_${DELTA}_uniform_inc-dec_${D}_${W}_${K}_${BITS_C}

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi


CUT_C=0
for ((i=$DELTA;i<=$END;i+=$DELTA));do
    ((CUT_C+=1))
done

((i-=$DELTA))
if [ "$i" -lt "$END" ]
then
    ((CUT_C+=1))
fi

CUT_D=0
((CUT_D=$DELTA * $BASE_OP))
echo CUT_D ${CUT_D}\; CUT_C ${CUT_C}

    CHSETTING $END $SETTING_SUF
    SETTING_FILE=${SETTING_DIR}/${SETTING}.dat
    echo $SETTING_FILE
    if [ -f $SETTING_FILE ]
    then
        rm $SETTING_FILE
    fi
    cp $TEMPLET $SETTING_FILE
    $(sed -i "9s/XXXK/${OP_K}K/" $SETTING_FILE)
    $(sed -i "10s/XXXK/$OP/" $SETTING_FILE)
    sh ./ycsb_workload_gen.sh $SETTING_FILE
    sh ./benchmark_creat.sh ${SETTING} $EXP_DIR reverse $CUT_D $CUT_C
    sh ./other_sketch_query_creat.sh ${SETTING} $EXP_DIR $D $W $K $BITS_C reverse $CUT_D $CUT_C
CLEAN