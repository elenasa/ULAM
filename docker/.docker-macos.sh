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
  DISPLAY - The DISPLAY environment variable to forward (Default: host.docker.internal:0)
  MOUNT_PATH - The path to mount to the containers home directory (Default: $HOME)
\0
EOF

WITH_DEFAULT DISPLAY "host.docker.internal:0"
unset X11_PATH
WITH_DEFAULT MOUNT_PATH "$HOME"

function run {
  docker run --rm -it \
         -e DISPLAY=host.docker.internal:0 \
         -v "$MOUNT_PATH:/home/$USER" \
         ulam-local /bin/bash
}
