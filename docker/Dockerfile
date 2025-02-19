FROM ubuntu:latest

RUN apt update -y && apt upgrade -y apt && apt install netcat-traditional -y && mkdir /app 
WORKDIR /app
ENTRYPOINT tail -f
