#!/bin/bash

script_dir="$(dirname "$0")"
variant=${variant:-build}
jobs=${jobs:-3}

source "${script_dir}/ubuntu-package-env.sh"
cd ${script_dir}/../..

configure_icl
echo "Build variant: $variant"

if [ "$variant" = "build" ]
then
  cd build
  make -j$jobs
elif [ "$variant" = "binary" ]
then
  mk-build-deps --install debian/control
  echo $HOSTNAME > ../docker_container_id.log
  debuild -b -uc -us
elif [ "$variant" = "source" ]
then
  gpg --passphrase-file ../packaging_passphrase.txt --batch  --import ../packaging.key
  debuild -S -i -I -sa -us -uc
  debsign \
    -p'gpg --batch --pinentry-mode=loopback --passphrase-file ../packaging_passphrase.txt' \
    -S ../icl_*.changes
  dput ppa:iclcv/icl ../icl_*.changes && rm ../icl_*
else
  echo "please specify a variant ('source' or 'binary')"
fi

