FROM ubuntu:18.04
RUN apt-get update
RUN apt-get -y install git wget build-essential \
                       libsdl-image1.2-dev libfreetype6-dev \
                       libssl-dev libsdl-ttf2.0-0 \
                       libcapture-tiny-perl \
                       libcrypt-openssl-rsa-perl \
                       libsdl-ttf2.0-dev \
                       libfile-sharedir-perl \
                       libfile-sharedir-install-perl

WORKDIR /usr/app
RUN git clone https://github.com/DaveAckley/MFM.git && \
    cd MFM && \
    git checkout v5.0.5 && \
    make

COPY . /usr/app/ULAM
RUN cd ULAM && \
    make -f rebuild.mk

WORKDIR /usr/app
RUN git clone https://github.com/DaveAckley/SPLAT.git && \
    cd SPLAT && \
    perl Makefile.PL && \
    make
    
RUN ln -s /usr/app/MFM/bin/mfms /usr/bin/mfms && \
    ln -s /usr/app/MFM/bin/mfmtest /usr/bin/mfmtest && \
    ln -s /usr/app/MFM/bin/mfzmake /usr/bin/mfzmake && \
    ln -s /usr/app/MFM/bin/mfzrun /usr/bin/mfzrun && \
    ln -s /usr/app/ULAM/bin/ulam /usr/bin/ulam && \
    ln -s /usr/app/SPLAT/blib/script/splattr /usr/bin/splattr
