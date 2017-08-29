FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

## Update (DO NOT upgrade) and install necessary packages.
RUN apt-get update && apt-get install -y \
        openjdk-8-jdk-headless openjdk-8-jre-headless ant \
	build-essential autoconf libtool swig2.0 libgmp-dev antlr3 libantlr3c-dev libboost-dev

COPY .jpf /root

## Get and build JPF-core.
COPY jpf-core /root/jpf-core
RUN cd /root/jpf-core && ant build

## Get and build JPF-symbc.
COPY jpf-symbc /root/jpf-symbc
RUN cd /root/jpf-symbc && ant build

## Set up environment
ENV PATH=/root/jpf-symbc/bin:$PATH
ENV LD_LIBRARY_PATH=/root/jpf-symbc/lib:$LD_LIBRARY_PATH
ENV TERM=dumb

## Prepare for interactive mode.
WORKDIR /root
ENTRYPOINT ["java", "-jar", "/root/jpf-core/build/RunJPF.jar", "+jpf-home=/root", "+jpf-core=/root/jpf-core", "+jpf-symbc=/root/jpf-symbc", "+extensions=${jpf-core},${jpf-symbc}"]
CMD []
