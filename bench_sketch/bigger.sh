#!/bin/bash
LEFT=$1
RIGHT=$2
total=$(sed -n '2p' $LEFT )
total=${total#*:}

echo $total

ARRAY1=($(awk '{print $0}' $LEFT ))
ARRAY2=($(awk '{print $0}' $RIGHT ))

j=0
for ((i=6;i<$total+6;i++))
do
    if [ ${ARRAY1[$i]} -lt ${ARRAY2[$i]} ]
    then
        let j++
        echo ${ARRAY1[$i]} ${ARRAY2[$i]}
    fi
done
echo ERROR: $j