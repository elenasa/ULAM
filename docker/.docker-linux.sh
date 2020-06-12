if [[ -z "$OS" ]]; then
  echo "This script should not be run directly, it is called by \`docker.sh\`. Try \`bash docker.sh\`."
  exit 1
fi

CMD=$0
read -r -d '\0' DOCUMENTATION <<EOF
ULAM / MFM Docker utility script

Usage:
  $CMD build - Builds the base Docker container
  $CMD build_local - Builds the local environment Docker container
  $CMD run - Builds all the necessary containers and launches the docker environment

Environment Variables:
  DISPLAY - The DISPLAY environment variable to forward (Default: $DISPLAY)
  X11_PATH - The path to the X11 Server (Default: /tmp/.X11-unix)
  MOUNT_PATH - The path to mount to the containers home directory (Default: $HOME)
\0
EOF

WITH_DEFAULT DISPLAY "$DISPLAY"
WITH_DEFAULT X11_PATH "/tmp/.X11-unix"
WITH_DEFAULT MOUNT_PATH "$HOME"

function run {
  # Launch the container
  CONTAINER_ID=$(docker run --rm -d \
                        --env="DISPLAY=$DISPLAY" \
                        --env="QT_X11_NO_MITSHM=1" \
                        --volume="$X11_PATH:/tmp/.X11-unix:rw" \
                        --volume="$MOUNT_PATH:/home/$USER" \
                        ulam-local /bin/bash -c "while true; do sleep 10; done")

  # Attach the XServer to the container and then connect to it via bash
  xhost "+local:$CONTAINER_ID"
  docker exec -it $CONTAINER_ID /bin/bash
  # Detatch the XServer
  xhost "-local:$CONTAINER_ID"

  # Kill the container
  docker kill $CONTAINER_ID
}
