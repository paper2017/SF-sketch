#!/bin/bash
echo TODO
#exit
LONG_BIT=$(getconf LONG_BIT)
D=$1
W=$2
K=$3
BITS_C=$4
START=$5
END=$6
DELTA=$7

#sh experiment_zipf_dec.sh 5 40000 3 24 11 8 1

#we create 100K items
BASE_ITEMS_K=100
BASE_OP_K=100
BASE_OP=100000

YCSB_HOME=/opt/ycsb-0.5.0-SNAPSHOT
DATABASE=basic

CUR_DIR=$(pwd)
PIDFILE=${CUR_DIR}/pid.tmp
EXP_DIR=${CUR_DIR}/raw_data_experiment
TEMPLET=${CUR_DIR}/templets/kv100K_opXXXK_zipf.dat
SETTING_DIR=${CUR_DIR}/settings
LOG_DIR=${CUR_DIR}/workingsets
REQ_DIR=${CUR_DIR}/requests


INIT_UP=init_up
INC_DEC_UP=inc-dec_up
QUERY=query


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
    rm $REQ_DIR/${SETTING_BASE}_init_up
    rm $REQ_DIR/${SETTING_BASE}_inc-dec_up
    rm $REQ_DIR/${SETTING_BASE}_query

    rm $LOG_DIR/${SETTING_BASE}_init_up.dat
    rm $LOG_DIR/${SETTING_BASE}_inc-dec_up.dat

    for ((i=$START-$DELTA;i>=$END;i-=$DELTA));do
        CHSETTING $DELTA dec$i
        rm $REQ_DIR/${SETTING}_inc-dec_up
#        rm $LOG_DIR/${SETTING}_inc-dec_up.dat
    done
}
######################SHOW USAGE#######################

if     [ "$#" -ne 7 ] || [ "$W" -lt 1 ] || [ "$D" -lt 1 ]\
    || [ "$K" -lt 1 ] || [ "${BITS_C}" -lt 1 ] || [ "${START}" -lt 0 ]\
    || [ "${END}" -gt "${START}" ] || [ "${END}" -lt 0 ] || [ "${DELTA}" -lt 1 ]
then
    echo -e We consider `RED 100K` items with frequents `RED "zipfianly distributed"`
    echo -e while in each experiment keep different frequent `RED DECREASED` by `COLOR DELTA``RED '(must > 0)'` from `COLOR START` to `COLOR END`
    echo -e on-chip, we use `COLOR W` buckets in each of the `COLOR D` tables, while using `COLOR BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR K*W` buckets in each of the `COLOR D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
    echo -e usage: ./experiment_uniform_dec.sh `COLOR D` `COLOR W` `COLOR K` `COLOR BITS_C` `COLOR START` `COLOR END` `COLOR DELTA`
#    echo -e `RED 'make sure START>=10 END >=5 START >= END'`
    exit
else
    echo -e We experiment `RED 100K` items with frequents `RED "zipfianly distributed"`
    echo -e while in each experiment keeping different frequent `RED DECREASED` by `COLOR $DELTA` from `COLOR $START` to `COLOR $END`
    echo -e on-chip, we use `COLOR $W` buckets in each of the `COLOR $D` tables, while using `COLOR $BITS_C` bits to record the estimated value of frequent
    echo -e off-chip, we use `COLOR $K*$W` buckets in each of the `COLOR $D` tables, while using `COLOR $LONG_BIT` bits to record the estimated value of frequent
fi


    CHSETTING $START inc
    SETTING_BASE=$SETTING
    SETTING_FILE=${SETTING_DIR}/${SETTING}.dat
    EXP_DIR=${CUR_DIR}/raw_data_experiment/kv${BASE_ITEMS_K}K_${START}_${END}_${DELTA}_zipf_dec_${D}_${W}_${K}_${BITS_C}
    if [ ! -d $EXP_DIR ];then
        mkdir $EXP_DIR
    fi
    echo $SETTING_FILE
    if [ -f $SETTING_FILE ]
    then
        rm $SETTING_FILE
    fi
    cp $TEMPLET $SETTING_FILE
    $(sed -i "9s/XXXK/${OP_K}K/" $SETTING_FILE)
    $(sed -i "10s/XXXK/$OP/" $SETTING_FILE)


######################GENERATING OPERATIONGS######################
echo using setting file $SETTING to create workingsets
echo generating ${SETTING}_${INIT_UP}, the init update used before benchmark
eval ${YCSB_HOME}/bin/ycsb load ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $SETTING_FILE > ${LOG_DIR}/${SETTING}_${INIT_UP}.dat

echo generating ${SETTING}_${INC_DEC_UP}, the increase/decrease update used before benchmark
eval ${YCSB_HOME}/bin/ycsb run ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $SETTING_FILE > ${LOG_DIR}/${SETTING}_${INC_DEC_UP}.dat

CUT_C=0
for ((i=$START-$DELTA;i>=$END;i-=$DELTA));do
#    CHSETTING $DELTA dec$i
#    eval ${YCSB_HOME}/bin/ycsb run ${DATABASE} -P ${YCSB_HOME}/workloads/workloadc -P $SETTING_FILE -p operationcount=$OP > ${LOG_DIR}/${SETTING}_${INC_DEC_UP}.dat
    ((CUT_C+=1))
done

echo generating requests/operations
echo open memcached service

rm ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}

/opt/memcached-master/bin/memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1
echo generate query operations
./bench_query_gen ${REQ_DIR}/${SETTING_BASE}_${QUERY} < ${LOG_DIR}/${SETTING_BASE}_${INIT_UP}.dat
echo generate init operations
./bench_update_gen -o ${REQ_DIR}/${SETTING_BASE}_${INIT_UP} -i ${LOG_DIR}/${SETTING_BASE}_${INIT_UP}.dat -l
echo generate inc/dec operations
./bench_update_gen -o ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP} -i ${LOG_DIR}/${SETTING_BASE}_${INC_DEC_UP}.dat

CUT_D=0
((CUT_D=$DELTA * $BASE_OP))
echo CUT_D ${CUT_D}\; CUT_C ${CUT_C}
./cut -f ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}  -c  $CUT_C -d $CUT_D

for ((i=$START-$DELTA, j=0;i>=$END;i-=$DELTA, j++));do
echo $j
    CHSETTING $DELTA dec$i
#    echo generate inc/dec operations
#    ./bench_update_gen -o ${REQ_DIR}/${SETTING}_${INC_DEC_UP} -i ${LOG_DIR}/${SETTING}_${INC_DEC_UP}.dat -d
    mv cut_${j}.out ${REQ_DIR}/${SETTING}_${INC_DEC_UP}
done

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE
echo memcached service closed


#######################CREATE BENCHMARK##########################
echo use workingset $SETTING_BASE to create benchmark

echo open memcached service

/opt/memcached-master/bin/memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1

echo create benchmark

./bench_client -s 127.0.0.1 -w $SETTING_BASE -l ${REQ_DIR}/${SETTING_BASE}_${INIT_UP}
./bench_client -s 127.0.0.1 -w $SETTING_BASE -l ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP}
./bench_client -s 127.0.0.1 -w $SETTING_BASE -l ${REQ_DIR}/${SETTING_BASE}_${QUERY}
mv ./benchmarks/${SETTING_BASE}.bench $EXP_DIR/${SETTING_BASE}.bench
for ((i=$START-$DELTA;i>=$END;i-=$DELTA));do
    CHSETTING $DELTA dec$i
    ./bench_client -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING}_${INC_DEC_UP}
    ./bench_client -s 127.0.0.1 -w $SETTING -l ${REQ_DIR}/${SETTING_BASE}_${QUERY}
    mv ./benchmarks/${SETTING}.bench $EXP_DIR/${SETTING}.bench
done

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE

echo memcached service closed


####################CREATE FCSKETCH.out, CMSKETCH.out etc...####################
sketch_DIR=${CUR_DIR%/*}/fcsketch/bin
sketch=${sketch_DIR}/fcsketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create fcsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP} ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        cp ${REQ_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR
        mv ${sketch_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}$j
    done
    $sketch -d $D -w $W -k $K -b $BITS_C -n $j
    mv $sketch_DIR/fcsketch.out $EXP_DIR/${SETTING_BASE}_fcsketch.out
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        mv $sketch_DIR/fcsketch$j.out $EXP_DIR/${SETTING}_fcsketch.out
        rm $sketch_DIR/${INC_DEC_UP}$j
    done
fi


sketch_DIR=${CUR_DIR%/*}/cmsketch/bin
sketch=${sketch_DIR}/cmsketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create cmsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP} ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        cp ${REQ_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR
        mv ${sketch_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j
    mv $sketch_DIR/cmsketch.out $EXP_DIR/${SETTING_BASE}_cmsketch.out
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        mv $sketch_DIR/cmsketch$j.out $EXP_DIR/${SETTING}_cmsketch.out
        rm $sketch_DIR/${INC_DEC_UP}$j
    done
fi


sketch_DIR=${CUR_DIR%/*}/csketch/bin
sketch=${sketch_DIR}/csketch
cd $sketch_DIR
echo use workingset $SETTING_BASE to create csketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${SETTING_BASE}_${INC_DEC_UP} ${REQ_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${SETTING_BASE}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${SETTING_BASE}_${QUERY} $sketch_DIR/${QUERY}
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        cp ${REQ_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR
        mv ${sketch_DIR}/${SETTING}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}$j
    done
    $sketch -d $D -w $W -b $BITS_C -n $j
    mv $sketch_DIR/csketch.out $EXP_DIR/${SETTING_BASE}_csketch.out
    for ((i=$START-$DELTA,j=0;i>=$END;i-=$DELTA));do
        ((j++))
        CHSETTING $DELTA dec$i
        mv $sketch_DIR/csketch$j.out $EXP_DIR/${SETTING}_csketch.out
        rm $sketch_DIR/${INC_DEC_UP}$j
    done
fi

cd $CUR_DIR

##################REMOVE INTERMEDIATE FILES#######################
#CLEAN
