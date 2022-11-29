#!/bin/bash

SSH_USER=my-ssh-user
SSH_PORT=22
SSH_SERVER=my-upgrade-server
UPGRADE_PROJECT_PATH=/home/${SSH_USER}/esp-update-server/projects/alraune
VERSION=$1

SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# Copy files to server
ssh -p ${SSH_PORT} ${SSH_USER}@${SSH_SERVER} mkdir -p ${UPGRADE_PROJECT_PATH}/${VERSION}
scp -P ${SSH_PORT} ${DIR}/data/Alraune/* ${SSH_USER}@${SSH_SERVER}:${UPGRADE_PROJECT_PATH}/${VERSION}/
scp -P ${SSH_PORT} ${DIR}/Alraune/Alraune.ino.bin.signed ${SSH_USER}@${SSH_SERVER}:${UPGRADE_PROJECT_PATH}/${VERSION}/firmware.bin

ssh -p ${SSH_PORT} ${SSH_USER}@${SSH_SERVER} ls -lah ${UPGRADE_PROJECT_PATH}/${VERSION}
