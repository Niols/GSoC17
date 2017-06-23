FROM ubuntu:16.04

MAINTAINER Nicolas `Niols` Jeannerod <niols@niols.fr>

## Update (DO NOT upgrade) and install necessary packages.
RUN apt-get update && apt-get install -y \
        openjdk-8-jdk-headless openjdk-8-jre-headless \
 	libz3-java \
 	mercurial ant \
	build-essential autoconf libtool swig2.0 libgmp-dev antlr3 libantlr3c-dev libboost-dev

COPY .jpf /root

## Get and build JPF-core.
COPY jpf-core /root/jpf-core
RUN cd /root/jpf-core && ant build

## Get and build CVC4.
COPY CVC4 /root/CVC4
RUN cd /root/CVC4 && ./autogen.sh
#RUN cd /root/CVC4 && sed -i 's|^.*jni\.h.*$|\0; ac_cv_header_jni_h=yes|' configure # So dirty
RUN cd /root/CVC4 && ./configure --enable-language-bindings=java JAVA_CPPFLAGS='-I/usr/lib/jvm/java-8-openjdk-amd64/include -I/usr/lib/jvm/java-8-openjdk-amd64/include/linux'
RUN cd /root/CVC4 && make
# #build?

## Get and build JPF-symbc.
COPY jpf-symbc /root/jpf-symbc
RUN cd /root/jpf-symbc && ant build

## Prepare for interactive mode.
WORKDIR /root
ENTRYPOINT ["java", "-jar", "/root/jpf-core/build/RunJPF.jar", "+jpf-home=/root", "+jpf-core=/root/jpf-core", "+jpf-symbc=/root/jpf-symbc", "+extensions=${jpf-core},${jpf-symbc}"]
CMD []
