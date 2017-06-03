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
USEMEMCACHED=$9

#sh experiment_real_inc.sh 5 40000 3 24 10 10 1 01 T

#we create 1000K items
BASE_ITEMS_K=1000
BASE_OP_K=1000
BASE_OP=1000000
#TEMPLET=./templets/kv100K_opXXXK_uniform.dat
#SETTING_DIR=./settings
#LOG_DIR=./workingsets

CUR_DIR=$(pwd)
PIDFILE=${CUR_DIR}/pid.tmp
EXP_DIR=${CUR_DIR}/raw_data_experiment
REQ_DIR=${CUR_DIR}/real_stream

#BLUE="\033[34m$BLUE\033[0m"


INC_DEC_UP=inc
QUERY=query

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
    SETTING=stream${STREAMID}_real_$2
}


CLEAN(){
#    for ((i=$START;i<=$END;i+=$DELTA));do
        CHSETTING $START 

#        rm $REQ_DIR/${SETTING}_init
        rm $REQ_DIR/${SETTING}inc
        rm $REQ_DIR/${SETTING}query

        
    for ((i=$START;i<$END;i+=$DELTA));do
        CHSETTING $DELTA ${INC_DEC_UP}$i
        rm $REQ_DIR/${SETTING}
#        rm $LOG_DIR/${SETTING}_inc-dec_up.dat
    done
#        rm $LOG_DIR/${SETTING}_init.dat
#        rm $LOG_DIR/${SETTING}_inc-dec_up.dat

#        rm ${SETTING_DIR}/${SETTING}.dat
#    done
}
#################################

if     [ "$#" -lt 8 ] || [ "$W" -lt 1 ] || [ "$D" -lt 1 ]\
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

#    echo usage: sh benchmark_creat.sh [setting] [EXP_DIR] [reverse] [delta] [# of delta]

SETTING_BASE=stream${STREAMID}_real

EXP_DIR=${CUR_DIR}/raw_data_experiment/${SETTING_BASE}_${INC_DEC_UP}_${D}_${W}_${K}_${BITS_C}_${START}_${END}_${DELTA}

PIDFILE=${CUR_DIR}/pid.tmp




if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

if [ ! -f ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP} ]
then
    echo We need Real Query Stream: ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}
    exit 0
fi



CUT_C=0
for ((i=$START;i<$END;i+=$DELTA));do
    ((CUT_C+=1))
done

CUT_D=0
((CUT_D=$DELTA * $BASE_OP))

CUT_S=0
((CUT_S=$START * $BASE_OP))

echo CUT_D ${CUT_D}\; CUT_C ${CUT_C}\; CUT_S ${CUT_S}
./cut -f ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}  -c  $CUT_C -d $CUT_D -s $CUT_S

mv cut_base.out ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base
for ((i=$START, j=0;i<$END;i+=$DELTA, j++));do
    CHSETTING $DELTA ${INC_DEC_UP}$i
    mv cut_${j}.out ${REQ_DIR}/${SETTING}
done

if [ -f $PIDFILE ]; then
    PID=$(cat $PIDFILE)
    kill $PID
rm $PIDFILE
fi

#######################CREATE BENCHMARK##########################
if [ ${USEMEMCACHED}S == TS ]; then

echo use workingset $SETTING_BASE to create benchmark

echo open memcached service

/opt/memcached-master/bin/memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1

echo create benchmark

./bench_client -s 127.0.0.1 -w $SETTING_BASE -l ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base
./bench_client -s 127.0.0.1 -w $SETTING_BASE -l ${REQ_DIR}/${SETTING_BASE}_${QUERY}
mv ./benchmarks/${SETTING_BASE}.bench $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0.bench

for ((i=$START;i<$END;i+=$DELTA));do
    CHSETTING $DELTA ${INC_DEC_UP}$i
    ./bench_client -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING}
    ./bench_client -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING_BASE}_${QUERY}
    mv ./benchmarks/${SETTING}.bench $EXP_DIR/${SETTING}.bench
done

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE

echo memcached service closed

fi  #${REQ_DIR}/${SETTING_BASE}_${QUERY}

EXP_RESULT_FILE=${CUR_DIR}/timetest/exp_result_time_tmp
touch ${EXP_RESULT_FILE}
echo " ">> ${EXP_RESULT_FILE}
echo ------------------------------------------>> ${EXP_RESULT_FILE}
echo  REAL INC -d $D -w $W -k $K -b $BITS_C >>  ${EXP_RESULT_FILE}

REQUEST_SUBFILE_PRE=inc-dec_up
####################CREATE FCSKETCH.out, CMSKETCH.out etc...####################
sketch_DIR=${CUR_DIR%/*}/fcsketch/bin
sketch=${sketch_DIR}/fcsketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create fcsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${REQUEST_SUBFILE_PRE} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base $sketch_DIR/${REQUEST_SUBFILE_PRE}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        cp ${REQ_DIR}/${SETTING} $sketch_DIR
        mv ${sketch_DIR}/${SETTING} $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
    $sketch -d $D -w $W -k $K -b $BITS_C -n $j -e ${EXP_RESULT_FILE}
    mv $sketch_DIR/fcsketch.out $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0_fcsketch.out
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        mv $sketch_DIR/fcsketch$j.out $EXP_DIR/${SETTING}_fcsketch.out
        rm $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
fi


sketch_DIR=${CUR_DIR%/*}/cmsketch/bin
sketch=${sketch_DIR}/cmsketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create cmsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${REQUEST_SUBFILE_PRE} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base $sketch_DIR/${REQUEST_SUBFILE_PRE}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        cp ${REQ_DIR}/${SETTING} $sketch_DIR
        mv ${sketch_DIR}/${SETTING} $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j -e ${EXP_RESULT_FILE}
    mv $sketch_DIR/cmsketch.out $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0_cmsketch.out
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        mv $sketch_DIR/cmsketch$j.out $EXP_DIR/${SETTING}_cmsketch.out
        rm $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
fi


sketch_DIR=${CUR_DIR%/*}/csketch/bin
sketch=${sketch_DIR}/csketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create csketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${REQUEST_SUBFILE_PRE} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base $sketch_DIR/${REQUEST_SUBFILE_PRE}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        cp ${REQ_DIR}/${SETTING} $sketch_DIR
        mv ${sketch_DIR}/${SETTING} $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j -e ${EXP_RESULT_FILE}
    mv $sketch_DIR/csketch.out $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0_csketch.out
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        mv $sketch_DIR/csketch$j.out $EXP_DIR/${SETTING}_csketch.out
        rm $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
fi

#if CML SuPporTed
if [ "$CML_SPT"a != a ]; then
sketch_DIR=${CUR_DIR%/*}/cmlsketch/bin
sketch=${sketch_DIR}/cmlsketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create cmlsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${REQUEST_SUBFILE_PRE} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base $sketch_DIR/${REQUEST_SUBFILE_PRE}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        cp ${REQ_DIR}/${SETTING} $sketch_DIR
        mv ${sketch_DIR}/${SETTING} $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j -e ${EXP_RESULT_FILE}
    mv $sketch_DIR/cmlsketch.out $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0_cmlsketch.out
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        mv $sketch_DIR/cmlsketch$j.out $EXP_DIR/${SETTING}_cmlsketch.out
        rm $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
fi
fi         #//support CML

sketch_DIR=${CUR_DIR%/*}/cmcusketch/bin
sketch=${sketch_DIR}/cmcusketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create cmcusketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${REQUEST_SUBFILE_PRE} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP}_base $sketch_DIR/${REQUEST_SUBFILE_PRE}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        cp ${REQ_DIR}/${SETTING} $sketch_DIR
        mv ${sketch_DIR}/${SETTING} $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j -e ${EXP_RESULT_FILE}
    mv $sketch_DIR/cmcusketch.out $EXP_DIR/${SETTING_BASE}_${INC_DEC_UP}0_cmcusketch.out
    for ((i=$START,j=0;i<$END;i+=$DELTA));do
        ((j++))
        CHSETTING $DELTA ${INC_DEC_UP}$i
        mv $sketch_DIR/cmcusketch$j.out $EXP_DIR/${SETTING}_cmcusketch.out
        rm $sketch_DIR/${REQUEST_SUBFILE_PRE}$j
    done
fi

cd $CUR_DIR





#CLEAN
