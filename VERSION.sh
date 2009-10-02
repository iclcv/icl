# this is just the version script
cat configure.ac | grep AC_INIT | cut -d ',' -f 2 | cut -c 2-6