FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

## Update (DO NOT upgrade) and install necessary packages.
RUN apt-get update && apt-get install -y \
        openjdk-8-jdk-headless openjdk-8-jre-headless \
	python build-essential autoconf libtool swig2.0 \
	libgmp-dev antlr3 libantlr3c-dev libboost-dev

## Get and build CVC4.

COPY . /root/CVC4

RUN cd /root/CVC4 && ./autogen.sh

RUN cd /root/CVC4 && ./configure \
        --enable-language-bindings=java \
	JAVA_CPPFLAGS='-I/usr/lib/jvm/java-8-openjdk-amd64/include -I/usr/lib/jvm/java-8-openjdk-amd64/include/linux'

RUN cd /root/CVC4 && make
RUN cd /root/CVC4 && make install
