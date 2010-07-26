#this  is just the version script
MAJOR=$(cat CMakeLists.txt | grep ICL_MAJOR | grep -v VERSION| cut -d \" -f 2);
MINOR=$(cat CMakeLists.txt | grep ICL_MINOR | grep -v VERSION| cut -d \" -f 2);
PATCH=$(cat CMakeLists.txt | grep ICL_PATCH | grep -v VERSION| cut -d \" -f 2);

echo ${MAJOR}.${MINOR}.${PATCH}
