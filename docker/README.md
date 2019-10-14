# ULAM for Docker

Goal: Run ULAM anywhere Docker can be run!

## Prerequisites

The only prerequisite is that you have docker. As of the time of writing:
```bash
$ docker --version
Docker version 18.09.0, build 4d60db4
```

### Installing Docker
* For Linux a bash script can be found here: https://get.docker.com/
* Mac / Windows: https://www.docker.com/get-started


## Quick Start

If you're on a Linux based system, you should be able to run `bash docker.sh run`. This does a few things:

1. Builds the Dockerfile in this directory and tags the container as `ulam:latest`.
2. Generates a file `.tmp/Dockerfile` using `ulam:latest` as a base. This container adds the current `$USER` to the container to ensure that read/writes happen properly between the container and the host environment. This container is tagged as `ulam-local:latest`.
3. Finally, launches `ulam-local` mounts the X11 Server to forward the display and also mounts `$HOME` for local read / write.

Step 1 may take some time as this compiles both MFM and ULAM. Luckily, once built, the container will only rebuild if changes are made to ULAM resulting in recompilation.

If all goes well, you should see a shell prompt with the containers build hash (e.g. `username@adaba8ed3aea:~$`). The container should be mounted to your `$HOME` directory. Run `ls` to verify.

* Launch MFM: `$ mfms`
* Launch ULAM: `$ ulam`
* Exit the container: `$ exit`

### Start a new ULAM project

```bash
mfzmake keygen $USER  # create $USER signing key for mfz files
mfzmake default $USER # set the default signing user to $USER
mkdir ~/MyElement/ && cd ~/MyElement/
ulam -i               # ulam starter code: code/MyElement.ulam
make run              # make the project and run the simulator
```

## Configuration
The `docker.sh` script comes with a very mild number of options. To see them, run `bash docker.sh`.

```bash
$ bash docker.sh
ULAM / MFM Docker utility script

Usage:
  docker.sh build - Builds the base Docker container
  docker.sh build_local - Builds the local environment Docker container
  docker.sh run - Builds all the necessary containers and launches the docker environment

Environment Variables:
  DISPLAY - The DISPLAY environment variable to forward (Default: :0)
  X11_PATH - The path to the X11 Server (Default: /tmp/.X11-unix)
  MOUNT_PATH - The path to mount to the containers home directory (Default: $HOME)
```

## Running on Windows 10

This container has been tested on Windows 10 using [VcXsrv](https://sourceforge.net/projects/vcxsrv/). There is a relatively useful tutorial for connecting docker containers to the display using Windows here: https://dev.to/darksmile92/run-gui-app-in-linux-docker-container-on-windows-host-4kde

### Running on Mac OS X

This was tested on Mac OS 10+ using [XQuartz](https://www.xquartz.org/). After installation, be sure to open XQuartz.app and verify under Settings > Privacy that you have enabled both "Allow connections from network clients" and "Authenticate connections" and restart XQuartz. You do not need to set `X11_PATH` if using the default `DISPLAY=host.docker.internal:0`.
