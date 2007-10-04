@ECHO OFF

:PREPARE
ECHO copy makefiles

copy ..\make\makefile.root ..\makefile

copy ..\make\makefile.root ..\ICLBlob\makefile
copy ..\make\makefile.root ..\ICLCC\makefile
copy ..\make\makefile.root ..\ICLCore\makefile
copy ..\make\makefile.root ..\ICLFilter\makefile
copy ..\make\makefile.root ..\ICLIO\makefile
copy ..\make\makefile.root ..\ICLMEx\makefile
copy ..\make\makefile.root ..\ICLQt\makefile
copy ..\make\makefile.root ..\ICLUtils\makefile
copy ..\make\makefile.root ..\ICLQuick\makefile
copy ..\make\makefile.root ..\ICLSVS\makefile

copy ..\make\makefile.src ..\ICLBlob\src\makefile
copy ..\make\makefile.src ..\ICLCC\src\makefile
copy ..\make\makefile.src ..\ICLCore\src\makefile
copy ..\make\makefile.src ..\ICLFilter\src\makefile
copy ..\make\makefile.src ..\ICLIO\src\makefile
copy ..\make\makefile.src ..\ICLMEx\src\makefile
copy ..\make\makefile.src ..\ICLQt\src\makefile
copy ..\make\makefile.src ..\ICLUtils\src\makefile
copy ..\make\makefile.src ..\ICLQuick\src\makefile
copy ..\make\makefile.src ..\ICLSVS\src\makefile

copy ..\make\makefile.examples ..\ICLBlob\examples\makefile
copy ..\make\makefile.examples ..\ICLCC\examples\makefile
::copy ..\make\makefile.examples ..\ICLCore\examples\makefile
copy ..\make\makefile.examples ..\ICLFilter\examples\makefile
copy ..\make\makefile.examples ..\ICLIO\examples\makefile
copy ..\make\makefile.examples ..\ICLMEx\examples\makefile
copy ..\make\makefile.examples ..\ICLQt\examples\makefile
copy ..\make\makefile.examples ..\ICLUtils\examples\makefile
copy ..\make\makefile.examples ..\ICLQuick\examples\makefile
copy ..\make\makefile.examples ..\ICLSVS\examples\makefile

copy ..\make\makefile.lib ..\ICLBlob\lib\makefile
copy ..\make\makefile.lib ..\ICLCC\lib\makefile
copy ..\make\makefile.lib ..\ICLCore\lib\makefile
copy ..\make\makefile.lib ..\ICLFilter\lib\makefile
copy ..\make\makefile.lib ..\ICLIO\lib\makefile
copy ..\make\makefile.lib ..\ICLMEx\lib\makefile
copy ..\make\makefile.lib ..\ICLQt\lib\makefile
copy ..\make\makefile.lib ..\ICLUtils\lib\makefile
copy ..\make\makefile.lib ..\ICLQuick\lib\makefile
copy ..\make\makefile.lib ..\ICLSVS\lib\makefile

copy ..\make\makefile.doc ..\ICLBlob\doc\makefile
copy ..\make\makefile.doc ..\ICLCC\doc\makefile
copy ..\make\makefile.doc ..\ICLCore\doc\makefile
copy ..\make\makefile.doc ..\ICLFilter\doc\makefile
copy ..\make\makefile.doc ..\ICLIO\doc\makefile
copy ..\make\makefile.doc ..\ICLMEx\doc\makefile
copy ..\make\makefile.doc ..\ICLQt\doc\makefile
copy ..\make\makefile.doc ..\ICLUtils\doc\makefile
copy ..\make\makefile.doc ..\ICLQuick\doc\makefile
copy ..\make\makefile.doc ..\ICLSVS\doc\makefile

copy ..\make\doxyfile ..\ICLBlob\doc\doxyfile
copy ..\make\doxyfile ..\ICLCC\doc\doxyfile
copy ..\make\doxyfile ..\ICLCore\doc\doxyfile
copy ..\make\doxyfile ..\ICLFilter\doc\doxyfile
copy ..\make\doxyfile ..\ICLIO\doc\doxyfile
copy ..\make\doxyfile ..\ICLMEx\doc\doxyfile
copy ..\make\doxyfile ..\ICLQt\doc\doxyfile
copy ..\make\doxyfile ..\ICLUtils\doc\doxyfile
copy ..\make\doxyfile ..\ICLQuick\doc\doxyfile
copy ..\make\doxyfile ..\ICLSVS\doc\doxyfile

ECHO ready to call make

