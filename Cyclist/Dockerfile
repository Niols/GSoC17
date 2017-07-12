FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

## Install prerequisites
RUN apt-get update && apt-get install -y \
     ocaml-nox ocaml-native-compilers \
     ocaml-melt libpcre-ocaml-dev libocamlgraph-ocaml-dev

## Get and install Spot
COPY spot-2.3.5.tar.gz /root
RUN cd /root && tar xzf spot-2.3.5.tar.gz
RUN cd /root/spot-2.3.5 && ./configure --disable-python
RUN cd /root/spot-2.3.5 && make
RUN cd /root/spot-2.3.5 && make install

## Get and install Cyclist
COPY cyclist-1f62148.tar.xz /root
RUN cd /root && tar xJf cyclist-1f62148.tar.xz
RUN cd /root/cyclist-1f62148 && make

## Prepare for extraction
COPY extract /root
