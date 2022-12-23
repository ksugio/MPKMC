FROM ubuntu:22.04

RUN apt update
RUN apt upgrade -y
RUN apt install -y --no-install-recommends \
    build-essential python3 python3-dev python3-pip \
    nano libfftw3-dev quantum-espresso
RUN pip3 install numpy
WORKDIR /MPKMC