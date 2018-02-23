#!/bin/bash

script_dir="$(dirname "$0")"
source "${script_dir}/ubuntu-package-env.sh"

ICL_OPTIONS+=" -DBUILD_WITH_IPP=ON -DBUILD_WITH_MKL=ON \
			   -DBUILD_WITH_IPP_OPTIONAL=ON -DBUILD_WITH_MKL_OPTIONAL=ON"

configure_icl
