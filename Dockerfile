FROM ubuntu:20.04

# Copy Cytosim executables
# COPY bin/sim ./bin


# Install necessary libraries
RUN apt-get update && apt-get install -y \
	build-essential \
	curl \
	libblas-dev \
	libhdf5-dev \
	liblapack-dev 

# # Setup shell script
WORKDIR home

COPY src ./src
COPY makefile .
COPY makefile.inc .
#COPY vary_compress_rate0002.cym .


RUN make report
RUN make sim
COPY docker.sh ./docker.sh
RUN chmod +x ./docker.sh

# Run sim and report
ENTRYPOINT ./docker.sh
