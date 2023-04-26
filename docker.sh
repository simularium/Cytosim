
#!/bin/bash

case ${SIMULATION_TYPE} in
	AWS)
		INPUT_FILE_PATH="${S3_INPUT_URL}${SIMULATION_NAME}/config/"
		OUTPUT_FILE_PATH="${S3_INPUT_URL}${SIMULATION_NAME}/outputs/${AWS_BATCH_JOB_ARRAY_INDEX:-0}/"
 		INPUT_FILE_NAME="${SIMULATION_NAME}${AWS_BATCH_JOB_ARRAY_INDEX:-0}.cym" 
 		aws s3 cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
	LOCAL)
		INPUT_FILE_PATH="/mnt/${SIMULATION_NAME}/config/"
		OUTPUT_FILE_PATH="/mnt/${SIMULATION_NAME}/outputs/${JOB_ARRAY_INDEX}/"
 		INPUT_FILE_NAME="${SIMULATION_NAME}${JOB_ARRAY_INDEX}.cym" 
 		cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
esac

cp bin/sim .
cp bin/report .
./sim config.cym
./report fiber:energy>fiber_energy.txt verbose=0
./report fiber:energy>fiber_energy_labels.txt

# Save output files
case ${SIMULATION_TYPE} in
	AWS)
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "*.cmo"
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "*.txt"
	;;
	LOCAL)
        mkdir -p $OUTPUT_FILE_PATH
		cp *.cmo $OUTPUT_FILE_PATH
		cp *.txt $OUTPUT_FILE_PATH
	;;
esac

#docker run --rm -e SIMULATION_NAME='varycompressrate' -e JOB_ARRAY_INDEX=0 -e SIMULATION_TYPE='LOCAL' -v /Users/karthik.vegesna/simularium-cytosim/Cytosim/:/mnt/ cytosim-arm:0.0.0 