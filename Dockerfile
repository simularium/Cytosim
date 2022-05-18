FROM ubuntu:20.04

RUN mkdir working
WORKDIR /working

# Copy Cytosim executables
COPY bin/sim .
# COPY bin/report .

# # Install necessary libraries
# RUN apt-get -y update
# RUN apt-get -y upgrade
# RUN apt-get -y install --no-install-recommends libx11-dev

# # Setup shell script
COPY docker.sh .
# RUN chmod +x ./docker.sh

# Run Sim
# ENTRYPOINT ["./docker.sh", "/bin/bash", "-l", "-c"]
# ENTRYPOINT ["./docker.sh"]
ENTRYPOINT ["./sim", "info"]
