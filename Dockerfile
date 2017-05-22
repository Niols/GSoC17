FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

ADD . /root

RUN apt-get update && apt-get upgrade -y

## Install necessary stuff to build and run JPF
RUN apt-get install -y openjdk-8-jdk-headless openjdk-8-jre-headless mercurial ant

## Get and build JPF-core
RUN cd /root/jpf-core && ant build

## GET and build JPF-symbc
RUN cd /root/jpf-symbc && ant build

## Prepare for interactive mode
WORKDIR /root
ENTRYPOINT ["java", "-jar", "/root/jpf-core/build/RunJPF.jar", "+jpf-home=/root", "+jpf-core=/root/jpf-core", "+jpf-symbc=/root/jpf-symbc", "+extensions=${jpf-core},${jpf-symbc}"]
CMD []
