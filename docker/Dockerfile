FROM ubuntu:16.04
RUN apt-get update
RUN apt-get -y install git wget build-essential \
                       libsdl-image1.2-dev libfreetype6-dev \
                       libssl-dev libsdl-ttf2.0-0 \
                       libcapture-tiny-perl \
                       libcrypt-openssl-rsa-perl

RUN wget https://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-2.0.11.tar.gz && \
    tar xfz SDL_ttf-2.0.11.tar.gz && \
    rm SDL_ttf-2.0.11.tar.gz && \
    cd SDL_ttf-2.0.11 && \
    ./configure && \
    make && \
    make install && \
    ln -s /usr/local/include/SDL/SDL_ttf.h /usr/include/SDL/SDL_ttf.h

WORKDIR /usr/app
RUN git clone https://github.com/DaveAckley/MFM.git && \
    cd MFM && \
    git checkout v5.0.3 && \
    make

COPY . /usr/app/ULAM
RUN cd ULAM && \
    make

RUN ln -s /usr/app/MFM/bin/mfms /usr/bin/mfms && \
    ln -s /usr/app/MFM/bin/mfmtest /usr/bin/mfmtest && \
    ln -s /usr/app/MFM/bin/mfzmake /usr/bin/mfzmake && \
    ln -s /usr/app/MFM/bin/mfzrun /usr/bin/mfzrun && \
    ln -s /usr/app/ULAM/bin/ulam /usr/bin/ulam
