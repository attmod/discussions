FROM ubuntu:20.04

# General container setup

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    apt-utils lsb-release gnupg2

RUN sh -c 'echo "deb http://www.icub.eu/ubuntu `lsb_release -cs` contrib/science" > /etc/apt/sources.list.d/icub.list' && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 57A5ACB6110576A6 && \
    apt-get update && apt-get install -y yarp

# Launch bash from /workdir
WORKDIR /workdir
CMD ["bash"]
