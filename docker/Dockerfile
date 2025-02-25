FROM ubuntu:latest

RUN apt update -y && apt upgrade -y apt && apt install netcat-traditional libcurl4-gnutls-dev libssl-dev g++ make -y && mkdir /app 
WORKDIR /app
ENTRYPOINT tail -f
