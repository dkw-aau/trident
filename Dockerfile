FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update -y
RUN apt install git g++ cmake libboost-all-dev liblz4-dev libtbb-dev libsparsehash-dev python3-dev libcurl4-gnutls-dev -y

COPY . /home
WORKDIR /home

RUN mkdir -p build
WORKDIR build/

RUN cmake -DSPARQL=1 -DSERVER=1 ..
RUN make

CMD ["./trident", "help"]