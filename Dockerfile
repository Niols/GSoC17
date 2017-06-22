FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

## Update (DO NOT upgrade) and install necessary packages.
RUN apt-get update && apt-get install -y \
        openjdk-8-jdk-headless openjdk-8-jre-headless \
	libz3-java \
	mercurial ant

COPY .jpf /root

## Get and build JPF-core.
COPY jpf-core /root/jpf-core
RUN cd /root/jpf-core && ant build

## GET and build JPF-symbc.
COPY jpf-symbc /root/jpf-symbc
RUN cd /root/jpf-symbc && ant build

## Prepare for interactive mode.
WORKDIR /root
ENTRYPOINT ["java", "-jar", "/root/jpf-core/build/RunJPF.jar", "+jpf-home=/root", "+jpf-core=/root/jpf-core", "+jpf-symbc=/root/jpf-symbc", "+extensions=${jpf-core},${jpf-symbc}"]
CMD []
