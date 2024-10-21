#Download base image latest debian
FROM debian:latest

# Update Software repository
RUN apt-get update

RUN apt-get install -y sudo && \
rm -rf /var/lib/apt/lists/*