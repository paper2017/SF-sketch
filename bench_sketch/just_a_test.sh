#!/bin/bash
D=5
W=40000
BITS_C=24
COUNT_AVG=100
echo Transforming configure from DWKB to FB for cqfilter
BITS_NSLOTS_22=22
BITS_REMAINDER_MIN=8
BITS_REMAINDER=
NUM_ITMES=100000
LOAD_FACTOR=95
NUM_SLOTS=
CAPACITY=
BLOATING_FACTOR=3
SLOTS_PER_BLOCK=64
QFBLOCK=17
BASE_ITEMS_K=200
TEMPLET=./templets/kv${BASE_ITEMS_K}K_opXXXK_uniform.dat
RED(){
    echo "\033[31m${1}\033[0m"
}
echo -e We consider `RED ${BASE_ITEMS_K}K` items with frequents `RED "uniformly distributed"`
echo ${TEMPLET} 
((NUM_SLOTS=$NUM_ITMES * $BLOATING_FACTOR * 100 / $LOAD_FACTOR))
echo NUM_SLOTS is $NUM_SLOTS
((CAPACITY=$D * $W * $BITS_C))
echo CAPACITY is $CAPACITY
#if (($NUM_SLOTS<$(echo "2^$BITS_NSLOTS_22" | bc -l))); then
#    NUM_SLOTS=$(echo "2^$BITS_NSLOTS_22" | bc -l)
#fi
NUM_BLOCKS=
((NUM_BLOCKS=($NUM_SLOTS + $SLOTS_PER_BLOCK - 1)/$SLOTS_PER_BLOCK))
echo NUM_BLOCKS is $NUM_BLOCKS
#if (($CAPACITY > $NUM_BLOCKS*($QFBLOCK * 8 + $SLOTS_PER_BLOCK * $BITS_REMAINDER_MIN))); then
#    ((BITS_REMAINDER=($CAPACITY / $NUM_BLOCKS - $QFBLOCK * 8)/$SLOTS_PER_BLOCK ))
#    BITS_NSLOTS=$(echo "l($NUM_SLOTS+1)/l(2)" | bc -l | sed "s/\..*//")
#else
    BITS_REMAINDER=$BITS_REMAINDER_MIN
    ((NUM_SLOTS=($CAPACITY / ($QFBLOCK * 8 + $SLOTS_PER_BLOCK *$BITS_REMAINDER_MIN )) * $SLOTS_PER_BLOCK ))

((NUM_BLOCKS=($NUM_SLOTS + $SLOTS_PER_BLOCK - 1)/$SLOTS_PER_BLOCK))
    echo NUM_SLOTS is $NUM_SLOTS, NUM_BLOCKS is $NUM_BLOCKS
    BITS_NSLOTS=$(echo "l($NUM_SLOTS-1)/l(2)+1" | bc -l | sed "s/\..*//")
#fi
echo bits for slots is $BITS_NSLOTS
echo bits for remainder is $BITS_REMAINDER
