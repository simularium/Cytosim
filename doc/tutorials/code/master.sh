#!/usr/bin/env bash

## Basically a script to create the data to make the exercise yourself

# Destroy any previous data (id there was any)

rm -rf ./scan

# Make the folder where all the simulation folders will be

mkdir scan

# Use preconfig.py to make a bunch of config files, you can change the number to the desired number of simulations

preconfig.py 128 ./config.cym.tpl scan

# Use collect to put all the config files in subfolders with formatted names

collect.py ./scan/run%04i/config.cym scan/config????.cym

# Run simulations in parallel (This will take a while)

scan.py ../sim nproc=4 scan/run????

# This is the step where the students have to make their own python script and analyse the data

