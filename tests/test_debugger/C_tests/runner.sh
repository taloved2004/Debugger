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


#creating folders

static_no_pie_out_folder="outputs/static/no-pie"
static_pie_out_folder="outputs/static/pie"
dynamic_no_pie_out_folder="outputs/dynamic/no-pie"
dynamic_pie_out_folder="outputs/dynamic/pie"

dir_list=($static_no_pie_out_folder $static_pie_out_folder $dynamic_no_pie_out_folder $dynamic_pie_out_folder)

for dir in "${dir_list[@]}"; do

	if [  -d "$dir" ]; then
     		rm -rf "$dir"
	fi
	mkdir "$dir"
done


echo "executing static tests"
for ((i=1; i<=6; i++ )); 
do
	../../../debug ./test_static_linking/no-pie/test${i}.out < inputs/inputs_static/in${i}.txt > ${static_no_pie_out_folder}/out${i}.txt
	../../../debug ./test_static_linking/pie/test${i}.out < inputs/inputs_static/in${i}.txt > ${static_pie_out_folder}/out${i}.txt
done

echo "executing dynamic tests"
for ((i=1; i<=5; i++ )); 
do
        ../../../debug ./test_dynamic_linking/no-pie/test${i} < inputs/inputs_dynamic/in${i}.txt > ${dynamic_no_pie_out_folder}/out${i}.txt
	../../../debug ./test_dynamic_linking/no-pie/test${i}_lazy < inputs/inputs_dynamic/in${i}.txt > ${dynamic_no_pie_out_folder}/out${i}_lazy.txt
	../../../debug ./test_dynamic_linking/pie/test${i} < inputs/inputs_dynamic/in${i}.txt > ${dynamic_pie_out_folder}/out${i}.txt
	../../../debug ./test_dynamic_linking/pie/test${i} < inputs/inputs_dynamic/in${i}.txt > ${dynamic_pie_out_folder}/out${i}_lazy.txt
done


echo 
echo "----------------"

echo "running diff - static"
    for (( i=1; i<=6; i++)); do
        diff "./exp_outs/static_outs/out${i}.txt" "./outputs/static/no-pie/out${i}.txt" &> /dev/null
        if [ $? -eq 0 ]; then
            echo -e "test ${i} - no-pie: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${i} - no-pie: ${RED}FAIL${NC}"
        fi
	diff "./exp_outs/static_outs/out${i}.txt" "./outputs/static/pie/out${i}.txt" &> /dev/null
	if [ $? -eq 0 ]; then
		echo -e "test ${i} -    pie: ${GREEN}PASS${NC}"
	else
		EXIT_STATUS=1
		echo -e "test ${i} -    pie: ${RED}FAIL${NC}"
    	fi
	done

echo
echo "-----------------------"

echo "running diff - dynamic"

for (( j=1; j<=5; j++)); do
	diff "./exp_outs/dynamic_outs/out${j}.txt" "./outputs/dynamic/no-pie/out${j}.txt" &> /dev/null
	if [ $? -eq 0 ]; then
            echo -e "test ${j} - no-pie: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${j} - no-pie: ${RED}FAIL${NC}"
        fi
	diff "./exp_outs/dynamic_outs/out${j}.txt" "./outputs/dynamic/pie/out${j}.txt" &> /dev/null
        if [ $? -eq 0 ]; then
            echo -e "test ${j} -    pie: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${j} -    pie: ${RED}FAIL${NC}"
        fi


    done

echo 
echo "----------------------"

echo "running diff - dynamic with lazy binding"
    for (( k=1; k<=5; k++)); do
	#lazy_binding tests
	diff "./exp_outs/dynamic_outs/out${k}.txt" "./outputs/dynamic/no-pie/out${k}_lazy.txt" &> /dev/null
      if [ $? -eq 0 ]; then
           echo -e "test ${k} - no-pie: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${k} - no-pie: ${RED}FAIL${NC}"
        fi
	diff "./exp_outs/dynamic_outs/out${k}.txt" "./outputs/dynamic/pie/out${k}_lazy.txt" &> /dev/null
        if [ $? -eq 0 ]; then
            echo -e "test ${k} -    pie: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${k} -    pie: ${RED}FAIL${NC}"
        fi

    done
	


exit ${EXIT_STATUS}
