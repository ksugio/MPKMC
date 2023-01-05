FROM ubuntu:18.04

RUN apt update
RUN apt upgrade -y
#RUN apt install -y tzdata && \
#    ln -sf /usr/share/zoneinfo/Asia/Tokyo /etc/localtime
#RUN apt install -y build-essential
RUN apt install -y python3 python3-dev python3-pip nano
#RUN apt install -y libfftw3-dev quantum-espresso
RUN apt install -y zlib1g zlib1g-dev
RUN pip3 install numpy
WORKDIR /MPKMC