# This file describes how to build seifnode into a runnable linux container
# with all dependencies installed.
#
# To build:
#
# 1) Install docker (http://docker.io)
# 2) Build: copy Dockerfile to an empty dir & run docker build -t $USER/seifnode .
# 3) Test: docker run $USER/seifnode node -e "console.log(require('seifnode'))"
#
# Note: Container does not automatically access hardware components for entropy
#       mining.

from ubuntu:16.04

MAINTAINER Aashish Sheshadri <aashish.sheshadri@gmail.com>
MAINTAINER Rohit Harchandani <harchu@gmail.com>

run apt-get update

RUN useradd -ms /bin/bash seif

# Installing OpenCV

run apt-get install -y cmake git subversion libopencv-dev libhighgui-dev


# Installing Portaudio

run apt-get install -y libasound-dev portaudio19-dev


# Installing crypto++

run svn checkout https://svn.code.sf.net/p/cryptopp/code/trunk/c5 cryptopp \
&& /bin/bash -c 'cd cryptopp; make static; make install PREFIX=/usr/local'


# Installing Node
# Note: Adds fs-extra to npm and replaces the fs.rename method with the fs.extra
#       move method that now automatically chooses what to do (rename/move).
#       https://github.com/npm/npm/issues/9863

run apt-get install -y nodejs npm && ln -s /usr/bin/nodejs /usr/local/bin/node
run cd usr/lib/nodejs/npm \
&& npm install fs-extra \
&& sed -i -e s/graceful-fs/fs-extra/ -e s/fs.rename/fs.move/ ./lib/utils/rename.js


# Installing patchelf

run echo "deb http://us.archive.ubuntu.com/ubuntu vivid main universe" >> /etc/apt/sources.list
run apt-get update
run apt-get install -y patchelf


USER seif
WORKDIR /home/seif


# Installing seifnode

run npm install nan
run npm install seifnode
