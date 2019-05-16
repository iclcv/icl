#!/bin/bash

script_dir="$(dirname "$0")"
source "${script_dir}/ubuntu-package-env.sh"

cd ${script_dir}/../..
variant=${variant:-test}
echo "Build variant: $variant"

#ICL_OPTIONS+=" -DBUILD_WITH_IPP=ON -DBUILD_WITH_MKL=ON "
configure_icl
echo $HOSTNAME > docker_container_id.log

if [ "$variant" = "binary" ]
then
  mk-build-deps --install debian/control
  debuild -b -uc -us
elif [ "$variant" = "source" ]
then
  mk-build-deps --install debian/control
  gpg --passphrase-file ../packaging_passphrase.txt --batch  --import ../packaging.key
  debuild -S -i -I -sa -us -uc
  debsign \
    -p'gpg --batch --pinentry-mode=loopback --passphrase-file ../packaging_passphrase.txt' \
    -S ../icl_*.changes
  dput ppa:iclcv/icl ../icl_*.changes && rm ../icl_*
elif [ "$variant" = "test" ]
then
  cd build
  make -j3
  make test
elif [ "$variant" = "pages" ]
then
  cd build
  make -j3
  make test
  make pages
else
  echo "please specify a variant ('source', 'test', 'pages' or 'binary')"
fi
