#!/bin/bash
CUR_DIR=$(pwd)
REQ_DIR=${CUR_DIR}/requests
EXP_DIR=${CUR_DIR}/raw_data_experiment
INIT=init
INIT_UP=init_up
INC_DEC_UP=inc-dec_up
QUERY=query

if [ "$#" -lt 6 ];then
    echo usage: sh other_sketch_query_creat.sh [{setting}] [{EXP_DIR}] [{D}] [{W}] [{K}] [{BITS_C}] [reverse/throughput] [# per interval] [# of intervals]
    exit 0
fi

setting=$1
#LONG_BIT=$(getconf LONG_BIT)
EXP_DIR=$2
D=$3
W=$4
K=$5
BITS_C=$6
DO_TIME_TEST=$7
ISREVERSE=$7
DELTA=$8
N_DELTA=$9

TIME_TEST_OPT=
if [ "$DO_TIME_TEST"a = TIMEa ] || [ "$DO_TIME_TEST"a = AGINGa ] ;then
	TIME_TEST_OPT='-T'
	echo DO TIME TEST
fi

if [ ! -f ${REQ_DIR}/${setting}_${INIT} ] || [ ! -f ${REQ_DIR}/${setting}_${INC_DEC_UP} ] || [ ! -f ${REQ_DIR}/${setting}_${QUERY} ]
then
    echo requests about setting: $setting are not generated
    exit 0
fi

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

EXP_RESULT_FILE=${CUR_DIR}/timetest/exp_result_time
touch ${EXP_RESULT_FILE}
echo " ">> ${EXP_RESULT_FILE}
echo ------------------------------------------>> ${EXP_RESULT_FILE}
echo  -d $D -w $W -k $K -b $BITS_C >>  ${EXP_RESULT_FILE}

echo Sketch Name: SF Sketch>> ${EXP_RESULT_FILE}
sketch_DIR=${CUR_DIR%/*}/sfsketch/bin
sketch=${sketch_DIR}/sfsketch
cd $sketch_DIR
echo use workingset $setting to create sfsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
    if [ "$ISREVERSE"a = reversea ];then
        $sketch -d $D -w $W -k $K -b $BITS_C -r $DELTA -e ${EXP_RESULT_FILE}
    elif [ "$ISREVERSE"a = throughputa ]; then
        $sketch -d $D -w $W -k $K -b $BITS_C -t -e ${EXP_RESULT_FILE}
    else
	echo Insert Query_fat Compose Query_slim Delete>>  ${EXP_RESULT_FILE}
	$sketch -d $D -w $W -k $K -b $BITS_C -e ${EXP_RESULT_FILE} ${TIME_TEST_OPT} 
	echo $sketch -d $D -w $W -k $K -b $BITS_C -e ${EXP_RESULT_FILE} ${TIME_TEST_OPT} 
	echo "  " >> ${EXP_RESULT_FILE}
    fi
fi


if [ "$ISREVERSE"a = reversea ];then
    for ((i=1;i<=$N_DELTA;i++))
    do
        mv $sketch_DIR/sfsketch_${i}.out $EXP_DIR/${setting}_${i}_sfsketch.out
    done
    for ((i=$N_DELTA-1;i>=0;i--))
    do
        mv $sketch_DIR/sfsketch_r_${i}.out $EXP_DIR/${setting}_r_${i}_sfsketch.out
    done
else
    mv $sketch_DIR/sfsketch.out $EXP_DIR/${setting}_sfsketch.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_sfsketch.out $EXP_DIR/${setting}_throughput_sfsketch.out
    fi
fi

if [ "$DO_TIME_TEST"a = AGINGa ];then
    exit
fi

cd $CUR_DIR

echo ------------------------------------------
echo New exp with CQF, temprarily exit.
echo Sketch Name: CQFilter >> ${EXP_RESULT_FILE}
sketch_DIR=${CUR_DIR%/*}/cqfilter/bin
sketch=${sketch_DIR}/cqfilter
cd $sketch_DIR

echo Transforming configure from DWKB to FB for cqfilter
BITS_NSLOTS_22=22
BITS_REMAINDER_MIN=6
BITS_REMAINDER=
NUM_ITMES=100000
LOAD_FACTOR=95
NUM_SLOTS=
CAPACITY=
BLOATING_FACTOR=3
SLOTS_PER_BLOCK=64
QFBLOCK=17
((NUM_SLOTS=$NUM_ITMES * $BLOATING_FACTOR * 100 / $LOAD_FACTOR))
echo NUM_SLOTS is $NUM_SLOTS
((CAPACITY=$D * $W * $BITS_C))
echo CAPACITY is $CAPACITY
#if (($NUM_SLOTS<$(echo "2^$BITS_NSLOTS_22" | bc -l))); then
#    NUM_SLOTS=$(echo "2^$BITS_NSLOTS_22" | bc -l)
#fi
NUM_BLOCKS=
((NUM_BLOCKS=($NUM_SLOTS + $SLOTS_PER_BLOCK - 1)/$SLOTS_PER_BLOCK))

#if (($CAPACITY > $NUM_BLOCKS*($QFBLOCK * 8 + $SLOTS_PER_BLOCK * $BITS_REMAINDER_MIN))); then
#    ((BITS_REMAINDER=($CAPACITY / $NUM_BLOCKS - $QFBLOCK * 8)/$SLOTS_PER_BLOCK ))
#    BITS_NSLOTS=$(echo "l($NUM_SLOTS+1)/l(2)" | bc -l | sed "s/\..*//")
#else
    BITS_REMAINDER=$BITS_REMAINDER_MIN
    ((NUM_SLOTS=($CAPACITY / ($QFBLOCK * 8 + $SLOTS_PER_BLOCK *$BITS_REMAINDER_MIN )) * $SLOTS_PER_BLOCK ))
    BITS_NSLOTS=$(echo "l($NUM_SLOTS-1)/l(2)+1" | bc -l | sed "s/\..*//")
#fi
echo bits for slots is $BITS_NSLOTS
echo bits for remainder is $BITS_REMAINDER
if [ "$ISREVERSE"a != reversea ]
then
    echo use workingset $setting to create cqfilter query result
    if [ -f $sketch ]
    then
        rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
        cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
        mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
        mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
        if [ "$ISREVERSE"a = throughputa ]; then
            $sketch -f $BITS_NSLOTS -b $BITS_REMAINDER -t
        else
echo Insert Query>>  ${EXP_RESULT_FILE}
        $sketch -f $BITS_NSLOTS -b $BITS_REMAINDER -e ${EXP_RESULT_FILE}  ${TIME_TEST_OPT}
	echo "  " >> ${EXP_RESULT_FILE}
        fi

    fi
    mv $sketch_DIR/cqfilter.out $EXP_DIR/${setting}_cqfilter.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_cqfilter.out $EXP_DIR/${setting}_throughput_cqfilter.out
    fi

fi
echo ------------------------------------------
exit

echo Sketch Name: CM Sketch>> ${EXP_RESULT_FILE}
sketch_DIR=${CUR_DIR%/*}/cmsketch/bin
sketch=${sketch_DIR}/cmsketch
cd $sketch_DIR
echo use workingset $setting to create cmsketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
    if [ "$ISREVERSE"a = reversea ];then
        $sketch -d $D -w $W -b $BITS_C -r $DELTA
    elif [ "$ISREVERSE"a = throughputa ]; then
        $sketch -d $D -w $W -b $BITS_C -t
    else
echo Insert Query Delete>>  ${EXP_RESULT_FILE}
        $sketch -d $D -w $W -b $BITS_C -e ${EXP_RESULT_FILE} ${TIME_TEST_OPT}
	echo "  " >> ${EXP_RESULT_FILE}
    fi
fi

if [ "$ISREVERSE"a = reversea ];then
    for ((i=1;i<=$N_DELTA;i++))
    do
        mv $sketch_DIR/cmsketch_${i}.out $EXP_DIR/${setting}_${i}_cmsketch.out
    done
    for ((i=$N_DELTA-1;i>=0;i--))
    do
        mv $sketch_DIR/cmsketch_r_${i}.out $EXP_DIR/${setting}_r_${i}_cmsketch.out
    done
else
    mv $sketch_DIR/cmsketch.out $EXP_DIR/${setting}_cmsketch.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_cmsketch.out $EXP_DIR/${setting}_throughput_cmsketch.out
    fi
fi

echo Sketch Name: CMCU Sketch>> ${EXP_RESULT_FILE}
sketch_DIR=${CUR_DIR%/*}/cmcusketch/bin
sketch=${sketch_DIR}/cmcusketch
cd $sketch_DIR

if [ "$ISREVERSE"a != reversea ]
then
    echo use workingset $setting to create cmcusketch query result
    if [ -f $sketch ]
    then
        rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
        cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
        mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
        mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
        if [ "$ISREVERSE"a = throughputa ]; then
            $sketch -d $D -w $W -b $BITS_C -t
        else
echo Insert Query>>  ${EXP_RESULT_FILE}
        $sketch -d $D -w $W -b $BITS_C -e ${EXP_RESULT_FILE}  ${TIME_TEST_OPT}
	echo "  " >> ${EXP_RESULT_FILE}
        fi

    fi
    mv $sketch_DIR/cmcusketch.out $EXP_DIR/${setting}_cmcusketch.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_cmcusketch.out $EXP_DIR/${setting}_throughput_cmcusketch.out
    fi

fi

echo Sketch Name: C Sketch>> ${EXP_RESULT_FILE}
sketch_DIR=${CUR_DIR%/*}/csketch/bin
sketch=${sketch_DIR}/csketch
cd $sketch_DIR
echo use workingset $setting to create csketch query result
if [ -f $sketch ]
then
    rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
    cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
    mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
    mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
    if [ "$ISREVERSE"a = reversea ];then
        $sketch -d $D -w $W -b $BITS_C -r $DELTA
    elif [ "$ISREVERSE"a = throughputa ]; then
        $sketch -d $D -w $W -b $BITS_C -t
    else
echo Insert Query Delete>>  ${EXP_RESULT_FILE}
        $sketch -d $D -w $W -b $BITS_C -e ${EXP_RESULT_FILE}  ${TIME_TEST_OPT}
	echo "  " >> ${EXP_RESULT_FILE}
    fi
fi

if [ "$ISREVERSE"a = reversea ];then
    for ((i=1;i<=$N_DELTA;i++))
    do
        mv $sketch_DIR/csketch_${i}.out $EXP_DIR/${setting}_${i}_csketch.out
    done
    for ((i=$N_DELTA-1;i>=0;i--))
    do
        mv $sketch_DIR/csketch_r_${i}.out $EXP_DIR/${setting}_r_${i}_csketch.out
    done
else
    mv $sketch_DIR/csketch.out $EXP_DIR/${setting}_csketch.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_csketch.out $EXP_DIR/${setting}_throughput_csketch.out
    fi
fi

#if CML SuPporTed
if [ "$CML_SPT"a != a ]; then
sketch_DIR=${CUR_DIR%/*}/cmlsketch/bin
sketch=${sketch_DIR}/cmlsketch
cd $sketch_DIR

if [ "$ISREVERSE"a != reversea ]
then
    echo use workingset $setting to create cmlsketch query result
    if [ -f $sketch ]
    then
        rm $sketch_DIR/${INC_DEC_UP} $sketch_DIR/${QUERY}
        cp ${REQ_DIR}/${setting}_${INC_DEC_UP} ${REQ_DIR}/${setting}_${QUERY} $sketch_DIR
        mv ${sketch_DIR}/${setting}_${INC_DEC_UP} $sketch_DIR/${INC_DEC_UP}
        mv ${sketch_DIR}/${setting}_${QUERY} $sketch_DIR/${QUERY}
        if [ "$ISREVERSE"a = throughputa ]; then
            $sketch -d $D -w $W -b $BITS_C -t
        else
            $sketch -d $D -w $W -b $BITS_C
        fi

    fi
    mv $sketch_DIR/cmlsketch.out $EXP_DIR/${setting}_cmlsketch.out
    if [ "$ISREVERSE"a = throughputa ]; then
        mv $sketch_DIR/throughput_cmlsketch.out $EXP_DIR/${setting}_throughput_cmlsketch.out
    fi

fi
fi         #//support CML

cd $CUR_DIR
