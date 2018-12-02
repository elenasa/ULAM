#!/bin/bash
# When successful, this script launches a docker container mounted to your home
# directory as the current $USER. Ideally, this allows you to install MFM / ULAM
# anywhere that docker works.
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

RED='\033[0;31m'
NC='\033[0m' # No Color

# Given a variable name and a value, checks to see if the variable is assigned.
# If it is not yet assigned, assigns it the specified value.
function WITH_DEFAULT {
  VAR_NAME=$1
  VALUE=$2
  if [[ -z ${!VAR_NAME} ]]; then
    eval "$VAR_NAME='$VALUE'"
  fi
}

WITH_DEFAULT OS "$(uname)"

case "$OS" in
  Linux) source $SCRIPT_DIR/.docker-linux.sh;;
  Darwin) source $SCRIPT_DIR/.docker-macos.sh;;
  *) (>&2 echo -e "${RED}Unknown Operating System $OS, defaulting to Linux.${NC}");
     source $SCRIPT_DIR/.docker-linux.sh;;
esac

# Build the base container which installs MFM and ULAM and adds them to the
# system path
function build {
  docker build -f $SCRIPT_DIR/Dockerfile -t ulam:latest "$SCRIPT_DIR/../"
}

function build_local {
  # Create a directory which will be ignored by git. This directory contains
  # your local Dockerfile information used for running an interactive shell
  mkdir -p "$SCRIPT_DIR/.tmp"

  # Creates the Dockerfile with the current USER cloned inside. This gives you
  # read / write access as yourself. Any files you create inside the container
  # will have proper permissions outside of the container.
  echo "FROM ulam:latest" > "$SCRIPT_DIR/.tmp/Dockerfile.local"
  echo "RUN useradd -ms /bin/bash $USER -u $UID" >> "$SCRIPT_DIR/.tmp/Dockerfile.local"
  echo "USER $USER" >> "$SCRIPT_DIR/.tmp/Dockerfile.local"
  echo "ENV USER=$USER" >> "$SCRIPT_DIR/.tmp/Dockerfile.local"
  echo "WORKDIR /home/$USER" >> "$SCRIPT_DIR/.tmp/Dockerfile.local"

  # Build your personal container
  docker build -f "$SCRIPT_DIR/.tmp/Dockerfile.local" -t ulam-local:latest "$SCRIPT_DIR/.tmp"
}

function go {
  build
  build_local
  run
}

ACTION=$1
case "$ACTION" in
  build) build;;
  build_local) build_local;;
  run) go;;
  *) echo "$DOCUMENTATION"
esac
