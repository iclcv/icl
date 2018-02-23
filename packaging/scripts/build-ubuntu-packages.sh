#!/bin/bash

script_dir="$(dirname "$0")"
source "${script_dir}/ubuntu-package-env.sh"
ICL_OPTIONS+=" -DBUILD_REDIST=DEB"

cd ${script_dir}/../..
variant=${variant:-binary}
echo "Build variant: $variant"

configure_icl
mk-build-deps --install debian/control
echo $HOSTNAME > ../docker_container_id.log

if [ "$variant" = "binary" ]
then
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
