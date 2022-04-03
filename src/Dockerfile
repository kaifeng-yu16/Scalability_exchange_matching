FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update
RUN apt-get install -y g++ make cgroup-tools postgresql postgresql-contrib libpqxx-dev 
RUN mkdir /code
WORKDIR /code
ADD . /code/
RUN chmod +x run.sh

