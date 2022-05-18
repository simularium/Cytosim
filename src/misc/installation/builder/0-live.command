#! /bin/bash
# This file is double-clickable on Mac-OSX
# Francois J. Nedelec, 8.10.2018

# go the the directory containing this script:

cd $(dirname $0) || exit 1;

# get number from name of script:

cmd=$(basename $0)
num=${cmd%-*}

# define executable and config from number:

exe=./play${num};
conf=config${num}.cym;

if [[ ! -x $exe ]]; then
	echo Error: missing executable $exe
	exit 1;
fi

# run live, directly in full screen

$exe live fullscreen=1 $conf;

# check status

if [ ! $? == 0 ]; then
    printf "\nCytosim did not terminate normally\n";
fi
