
#!/bin/bash

# Load input files 
case ${SIMULATION_TYPE} in
	AWS)
		INPUT_FILE_PATH="${BATCH_WORKING_URL}${FILE_SET_NAME}/config/"
		OUTPUT_FILE_PATH="${BATCH_WORKING_URL}${FILE_SET_NAME}/outputs/${AWS_BATCH_JOB_ARRAY_INDEX:-0}/"
 		INPUT_FILE_NAME="${FILE_SET_NAME}_${AWS_BATCH_JOB_ARRAY_INDEX:-0}.cym" 
 		aws s3 cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
	LOCAL)
		INPUT_FILE_PATH="/mnt/${SIMULATION_NAME}/config/"
		OUTPUT_FILE_PATH="/mnt/${SIMULATION_NAME}/outputs/${JOB_ARRAY_INDEX}/"
 		INPUT_FILE_NAME="${SIMULATION_NAME}_${JOB_ARRAY_INDEX}.cym" 
 		cp $INPUT_FILE_PATH$INPUT_FILE_NAME config.cym
	;;
esac

echo $INPUT_FILE_PATH
echo $OUTPUT_FILE_PATH
echo $INPUT_FILE_NAME

# Copy over sim and report to home
cp bin/sim .
cp bin/report .

# Run sim and report fiber energy 
./sim + config.cym 
./report fiber:energy>fiber_energy.txt verbose=0
./report fiber:energy>fiber_energy_labels.txt
./report fiber:segment_energy>fiber_segment_curvature.txt
./report fiber:points> fiber_points.txt
./report single:position> singles.txt

# Save output filess
case ${SIMULATION_TYPE} in
	AWS)
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "objects.cmo" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "messages.cmo" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "properties.cmo" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "fiber_energy.txt" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "fiber_energy_labels.txt" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "fiber_segment_curvature.txt" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "fiber_points.txt" 
		aws s3 cp . $OUTPUT_FILE_PATH --recursive --exclude "*" --include "singles.txt" 
	;;
	LOCAL)
        mkdir -p $OUTPUT_FILE_PATH
		cp *.cmo $OUTPUT_FILE_PATH
		cp *.txt $OUTPUT_FILE_PATH
	;;
esac
