#!/bin/bash

# Run the model
cp ../bin/sim .
cp ../bin/report .
./sim
./report fiber:points > fiber_points.txt
./report single:force > single_force.txt
rm sim
rm report
