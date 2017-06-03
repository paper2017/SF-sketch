#!/bin/bash
LONG_BIT=$(getconf LONG_BIT)
D=5
W=40000
K=3
BITS_C=24
START=$1
END=$1
DELTA=1
KEYLEN=$2

#sh experiment_uniform_inc.sh 5 40000 3 24 1 100 1

#we create 100K items
BASE_ITEMS_K=1
BASE_OP_K=1
BASE_OP=1000000
TEMPLET=./templets/kvXX_OPXX.dat
SETTING_DIR=./settings
LOG_DIR=./workingsets
REQ_DIR=./requests
RAWDATA_DIR=./rawdata4hash
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
    SETTING=op${OP_K}K_$2
}


CLEAN(){
    for ((i=$START;i<=$END;i+=$DELTA));do
        CHSETTING $i inc

        rm $REQ_DIR/${SETTING}_init
        rm $REQ_DIR/${SETTING}_inc-dec_up
        rm $REQ_DIR/${SETTING}_query

        rm $LOG_DIR/${SETTING}_init.dat
        rm $LOG_DIR/${SETTING}_inc-dec_up.dat

        rm ${SETTING_DIR}/${SETTING}.dat
    done
}
#################################

if     [ "$#" -ne 2 ]
then
    echo  usage: ./data4hash.sh  "[#](#Mi of counts)" "[a|b|c](key length type)"
    exit
fi

#EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${START}_${END}_${DELTA}_uniform_inc_${D}_${W}_${K}_${BITS_C}

#if [ ! -d $EXP_DIR ];then
#    mkdir $EXP_DIR
#fi

for ((i=$START;i<=$END;i+=$DELTA))
do
    CHSETTING $i inc
    SETTING_FILE=${SETTING_DIR}/${SETTING}.dat
    echo $SETTING_FILE
    if [ -f $SETTING_FILE ]
    then
        rm $SETTING_FILE
    fi
    cp $TEMPLET $SETTING_FILE
    $(sed -i "5s/XXXMi/${OP_K}K/" $SETTING_FILE)
    $(sed -i "6s/XXXMi/$OP/" $SETTING_FILE)
    echo $KEYLEN
    for ((j=1;j<=3;j++))
    do
        sh ./data4hash_gen2.sh $SETTING_FILE $KEYLEN
        cp ./${REQ_DIR}/RAWDATA_${KEYLEN} ./${RAWDATA_DIR}/RAWDATA_${KEYLEN}_${i}_${j}.dat
    done
#    sh ./benchmark_creat.sh ${SETTING} $EXP_DIR
#    sh ./other_sketch_query_creat.sh ${SETTING} $EXP_DIR $D $W $K $BITS_C
done

#CLEAN