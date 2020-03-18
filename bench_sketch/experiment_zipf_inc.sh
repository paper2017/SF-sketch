#!/bin/bash
LONG_BIT=$(getconf LONG_BIT)
#we create ${BASE_ITEMS_K}K items
BASE_ITEMS_K=$1
D=$2
W=$3
K=$4
BITS_C=$5
START=$6
END=$7
DELTA=$8
DO_TIME_TEST=$9
DO_GEN=$9


#sh experiment_zipf_inc.sh 100 5 40000 3 24 100 100 1 TIME
#sh experiment_zipf_inc.sh 100 5 40000 3 24 100 100 1 GEN 
#sh experiment_zipf_inc.sh 100 5 40000 3 24 100 100 1 BEN

BASE_OP_K=100
BASE_OP=100000
TEMPLET=./templets/kvXXXK_opXXXK_zipf.dat
SETTING_DIR=./settings
LOG_DIR=./workingsets
REQ_DIR=./requests
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
#    SETTING=kv${BASE_ITEMS_K}K_op${OP_K}K_zipf_$2_${D}_${W}_${K}_${BITS_C}
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

if     [ "$#" -lt 8 ] || [ "$W" -lt 1 ] || [ "$D" -lt 1 ]\
    || [ "$K" -lt 1 ] || [ "${BITS_C}" -lt 1 ] || [ "${START}" -lt 1 ]\
    || [ "${END}" -lt "${START}" ] || [ "${DELTA}" -lt 1 ]
then
    echo -e We consider `RED ${BASE_ITEMS_K}K` items with frequents obeying `RED "zipfian distribution"`
    echo -e while in each experiment keep different frequent `RED INCREASED` by `COLOR DELTA``RED '(must > 0)'` from `COLOR START` to `COLOR END`
    echo -e on-chip, we use `COLOR W` buckets in each of the `COLOR D` tables, while using `COLOR BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR K*W` buckets in each of the `COLOR D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
    echo -e usage: ./experiment_zipf_inc.sh `COLOR D` `COLOR W` `COLOR K` `COLOR BITS_C` `COLOR START` `COLOR END` `COLOR DELTA`
    exit
else
    echo -e We experiment `COLOR ${BASE_ITEMS_K}K` items with frequents obeying `RED "zipfian distribution"`
    echo -e while in each experiment keeping different frequent `RED INCREASED` by `COLOR $DELTA` from `COLOR $START` to `COLOR $END`
    echo -e on-chip, we use `COLOR $W` buckets in each of the `COLOR $D` tables, while using `COLOR $BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR $K*$W` buckets in each of the `COLOR $D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
fi

if [ "$DO_TIME_TEST"a != TIMEa ];then
	EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${START}_${END}_${DELTA}_zipf_inc_${D}_${W}_${K}_${BITS_C}
else
	EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${START}_${END}_${DELTA}_zipf_inc_${D}_${W}_${K}_${BITS_C}_${DO_TIME_TEST}
fi

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

###TEST BEGIN
#    ((OP_XK=2 * $BASE_OP_K))
#    ((OP_X=2 * $BASE_OP))
###TEST END

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
    $(sed -i "7s/XXXK/${BASE_ITEMS_K}K/" $SETTING_FILE)
    $(sed -i "8s/XXX/${BASE_ITEMS_K}/" $SETTING_FILE)
    $(sed -i "9s/XXXK/${OP_K}K/" $SETTING_FILE)
    $(sed -i "10s/XXXK/$OP/" $SETTING_FILE)
####TEST BEGIN
#    $(sed -i "9s/XXXK/${OP_XK}K/" $SETTING_FILE)
#    $(sed -i "10s/XXXK/$OP_X/" $SETTING_FILE)
####TEST END
    if [ "$DO_GEN"a = GENa ];then
    	sh ./ycsb_workload_gen.sh $SETTING_FILE
    	sh ./benchmark_creat.sh ${SETTING} $EXP_DIR
    elif [ "$DO_GEN"a = BENa ];then
    	sh ./benchmark_creat.sh ${SETTING} $EXP_DIR
    fi
    sh ./other_sketch_query_creat.sh ${SETTING} $EXP_DIR $D $W $K $BITS_C $DO_TIME_TEST
done

#CLEAN
