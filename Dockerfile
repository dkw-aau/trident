FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive

COPY . /home
WORKDIR /home

RUN apt update -y
RUN apt install git g++ cmake libboost-all-dev liblz4-dev libtbb-dev libsparsehash-dev python3-dev libcurl4-gnutls-dev -y
RUN mkdir build
WORKDIR build/
RUN cmake ..
RUN make

CMD ["./trident", "help"]