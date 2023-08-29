#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

EXIT_STATUS=0

chmod +x ./compile.sh

echo "compiling files"
echo
echo ------------------

./compile.sh

echo "executing static tests"
for ((i=1; i<=6; i++ )); 
do
	../../debug ./test_static_linking/test${i}.out < inputs/inputs_static/in${i}.out > outputs/static/out${i}.txt
done

echo "executing dynamic tests"
for ((i=1; i<=5; i++ )); 
do
        ../../debug ./test_dynamic_linking/test${i} < inputs/inputs_dynamic/in${i}.out > outputs/dynamic/out${i}.txt
	../../debug ./test_dynamic_linking/test${i}_lazy < inputs/inputs_dynamic/in${i}.out > outputs/dynamic/out${i}_lazy.txt
done


echo 
echo "----------------"

echo "running diff - static"
    for (( i=1; i<=6; i++)); do
        diff "./exp_outs/static_outs/out${i}.txt" "./outputs/static/out${i}.txt" &> /dev/null
        if [ $? -eq 0 ]; then
            echo -e "test ${i}: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${i}: ${RED}FAIL${NC}"
        fi
    done

echo
echo "-----------------------"

echo "running diff - dynamic"

for (( j=1; j<=5; j++)); do
diff "./exp_outs/dynamic_outs/out${j}.txt" "./outputs/dynamic/out${j}.txt" &> /dev/null
	if [ $? -eq 0 ]; then
            echo -e "test ${j}: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${j}: ${RED}FAIL${NC}"
        fi
    done

echo 
echo "----------------------"

echo "running diff - dynamic with lazy binding"
    for (( k=1; k<=5; k++)); do
	#lazy_binding tests
	diff "./exp_outs/dynamic_outs/out${k}.txt" "./outputs/dynamic/out${k}_lazy.txt" &> /dev/null
      if [ $? -eq 0 ]; then
           echo -e "test ${k} - lazy: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${k} - lazy: ${RED}FAIL${NC}"
        fi
    done
	


exit ${EXIT_STATUS}
