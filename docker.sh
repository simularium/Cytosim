#!/bin/bash

# Load input files
case ${SIMULATION_TYPE} in
	AWS)
		INPUT_FILE_PATH="${BATCH_WORKING_URL}configs/"
		OUTPUT_FILE_PATH="${BATCH_WORKING_URL}outputs/"
 		INPUT_FILE_NAME="${FILE_SET_NAME}_${AWS_BATCH_JOB_ARRAY_INDEX:-0}.cym"
 		aws s3 cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
	LOCAL)
		INPUT_FILE_PATH="/mnt/configs/"
		OUTPUT_FILE_PATH="/mnt/outputs/"
 		INPUT_FILE_NAME="${SIMULATION_NAME}_${JOB_ARRAY_INDEX}.cym"
 		cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
esac

# Copy over sim and report to home
cp bin/sim .
cp bin/report .

# Run sim
./sim + config.cym

# Run report
mkdir reports
./report fiber:energy > reports/fiber_energy.txt verbose=0
./report fiber:energy > reports/fiber_energy_labels.txt
./report fiber:segment_energy > reports/fiber_segment_curvature.txt
./report fiber:points > reports/fiber_points.txt
./report single:position > reports/singles.txt

# Save output filess
case ${SIMULATION_TYPE} in
	AWS)
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "*.cmo"
		aws s3 cp reports/ $OUTPUT_FILE_PATH --recursive
	;;
	LOCAL)
        mkdir -p $OUTPUT_FILE_PATH
		cp *.cmo $OUTPUT_FILE_PATH
		cp *.txt $OUTPUT_FILE_PATH
	;;
esac
