FROM ubuntu:20.04

# Install necessary libraries
RUN apt-get update && apt-get install -y \
	build-essential \
	curl \
	libblas-dev \
	libhdf5-dev \
	liblapack-dev 

# Change working directory
WORKDIR home

# Copy over source files and makefiles
COPY src ./src
COPY makefile .
COPY makefile.inc .

# Make report and sim
RUN make report
RUN make sim

# Copy over entrypoint script and make executable
COPY docker.sh ./docker.sh
RUN chmod +x ./docker.sh

# Run entrypoint script
ENTRYPOINT ./docker.sh
