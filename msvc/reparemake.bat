@ECHO OFF

:PREPARE
ECHO remove all copied makefiles

del  ..\makefile

del  ..\ICLBlob\makefile
del  ..\ICLCC\makefile
del  ..\ICLCore\makefile
del  ..\ICLFilter\makefile
del  ..\ICLIO\makefile
del  ..\ICLMEx\makefile
del  ..\ICLQt\makefile
del  ..\ICLUtils\makefile
del  ..\ICLQuick\makefile
del  ..\ICLSVS\makefile

del  ..\ICLBlob\src\makefile
del  ..\ICLCC\src\makefile
del  ..\ICLCore\src\makefile
del  ..\ICLFilter\src\makefile
del  ..\ICLIO\src\makefile
del  ..\ICLMEx\src\makefile
del  ..\ICLQt\src\makefile
del  ..\ICLUtils\src\makefile
del  ..\ICLQuick\src\makefile
del  ..\ICLSVS\src\makefile

del  ..\ICLBlob\examples\makefile
del  ..\ICLCC\examples\makefile
::del  ..\ICLCore\examples\makefile
del  ..\ICLFilter\examples\makefile
del  ..\ICLIO\examples\makefile
del  ..\ICLMEx\examples\makefile
del  ..\ICLQt\examples\makefile
del  ..\ICLUtils\examples\makefile
del  ..\ICLQuick\examples\makefile
del  ..\ICLSVS\examples\makefile

del  ..\ICLBlob\lib\makefile
del  ..\ICLCC\lib\makefile
del  ..\ICLCore\lib\makefile
del  ..\ICLFilter\lib\makefile
del  ..\ICLIO\lib\makefile
del  ..\ICLMEx\lib\makefile
del  ..\ICLQt\lib\makefile
del  ..\ICLUtils\lib\makefile
del  ..\ICLQuick\lib\makefile
del  ..\ICLSVS\lib\makefile

del  ..\ICLBlob\doc\makefile
del  ..\ICLCC\doc\makefile
del  ..\ICLCore\doc\makefile
del  ..\ICLFilter\doc\makefile
del  ..\ICLIO\doc\makefile
del  ..\ICLMEx\doc\makefile
del  ..\ICLQt\doc\makefile
del  ..\ICLUtils\doc\makefile
del  ..\ICLQuick\doc\makefile
del  ..\ICLSVS\doc\makefile

del  ..\ICLBlob\doc\doxyfile
del  ..\ICLCC\doc\doxyfile
del  ..\ICLCore\doc\doxyfile
del  ..\ICLFilter\doc\doxyfile
del  ..\ICLIO\doc\doxyfile
del  ..\ICLMEx\doc\doxyfile
del  ..\ICLQt\doc\doxyfile
del  ..\ICLUtils\doc\doxyfile
del  ..\ICLQuick\doc\doxyfile
del  ..\ICLSVS\doc\doxyfile

ECHO deleted all linked files...ready for svn update

