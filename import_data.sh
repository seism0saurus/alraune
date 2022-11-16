#!/bin/bash

VERSION=$1

SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# gzip and copy files
cd ${DIR}/data
find ./ -type f -exec bash -c "gzip --best -k {}; mv {}.gz ${DIR}/Alraune/data/" \;

# ca file should not be gziped
rm ${DIR}/Alraune/data/ca.cer.gz
cp ca.cer ${DIR}/Alraune/data/

# config file should not be gziped
rm ${DIR}/Alraune/data/config.json.gz
cp config.json ${DIR}/Alraune/data/

# public key for signature verification should not be gziped
rm ${DIR}/data/Alraune/public.key.gz
cp public.key.gz ${DIR}/Alraune/data/

# mp3 file should not be gziped
rm ${DIR}/Alraune/data/tickle.mp3.gz
cp tickle.mp3 ${DIR}/Alraune/data/

cd ${DIR}/Alraune/data
ls -l
