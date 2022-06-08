FROM ubuntu:20.04

# Copy Cytosim executables
COPY bin/sim ./bin
COPY bin/report ./bin

# Install necessary libraries
RUN apt-get update && apt-get install -y \
	build-essential \
	curl \
	libblas-dev \
	libhdf5-dev \
	liblapack-dev

# Setup shell script
WORKDIR home
COPY docker.sh /docker.sh
RUN chmod +x /docker.sh

# Run sim and report
ENTRYPOINT /docker.sh
