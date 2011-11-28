# general
FOLDER:=$(notdir $(shell pwd))

CXX=g++
LD=g++
DEP=g++
WL_RP=-Wl,-rpath=

PWD=`pwd`

# files
SOURCES=$(wildcard src/*.cpp)
SOURCES:=$(filter-out $(wildcard src/*_moc.cpp),${SOURCES})
OBJS=$(patsubst src/%.cpp,obj/%.o,${SOURCES})
ifeq "${OBJS}" ""
	LIB_EMPTY_INDICATOR="(empty)"
endif

APP_SOURCES=$(wildcard app/*.cpp)
APPS_NO_PATH=$(patsubst app/%.cpp,%,${APP_SOURCES})
APPS=$(patsubst app/%.cpp,bin/%,${APP_SOURCES})


DEP_FILES:=$(patsubst src/%.cpp,obj/%.src.dep,${SOURCES})
APP_DEP_FILES:=$(patsubst app/%.cpp,obj/%.app.dep,${APP_SOURCES})

DEP_FILES:=${DEP_FILES} ${APP_DEP_FILES}

TEST=./test/runner
TEST_SOURCES=$(wildcard test/*.cpp)

# avoid single space in dep-files string!
ifeq "${DEP_FILES}" " "
DEP_FILES=
endif

all:

# include custom makefile
-include Makefile.custom

LIB=${PROJECT_NAME}
LIB_FILE=lib${LIB}.so
LIB_LNK=-Llib -l${LIB}
PC=$(patsubst %,%.pc,${LIB})


# basic flags
CXXFLAGS:=${CXXFLAGS} -Isrc -Iapp -fPIC -Wall
#CXXCPP=
#LDFLAGS=


# extract quit flag
ifeq "${QUIET}" "TRUE"
AT=@
else
AT=
endif

ifeq "${COLORED}" "TRUE"
FBLA='\033[30m'
FRED='\033[31m'
FGRE='\033[32m'
FYEL='\033[33m'
FBLU='\033[34m'
FPUR='\033[35m'
FCYA='\033[36m'

BBLA='\033[40m'
BRED='\033[41m'
BGRE='\033[42m'
BYEL='\033[43m'
BBLU='\033[44m'
BPUR='\033[45m'
BCYA='\033[46m'

BBLI='\033[5m'
INV='\033[7m'
DEF='\033[0m'
endif

# other helpers
LIB_INSTALL_PREFIX=${INSTALL_PREFIX}/lib

INC_INSTALL_PREFIX=${INSTALL_PREFIX}/include/${PROJECT_NAME}
DOC_INSTALL_PREFIX=${INSTALL_PREFIX}/doc/${PROJECT_NAME}
BIN_INSTALL_PREFIX=${INSTALL_PREFIX}/bin/${PROJECT_NAME}
ETC_INSTALL_PREFIX=${INSTALL_PREFIX}/etc/${PROJECT_NAME}
REL_BIN_INSTALL_PREFIX=bin/${PROJECT_NAME}


APPS_INSTALLED=$(patsubst app/%.cpp,${BIN_INSTALL_PREFIX}/%,${APP_SOURCES})

REL_INC_INSTALL_PREFIX=$(patsubst ${INSTALL_PREFIX}/%,%,${INC_INSTALL_PREFIX})

ECHO_TEST_RESULT=$(shell echo -e test > .testfile && cat .testfile && rm .testfile)

ifeq  "${ECHO_TEST_RESULT}" "test" 
ECHO=@echo -e
ECHO_NO_AT=echo -e
else
ECHO=@echo
ECHO_NO_AT=echo
endif

INSTALL=${AT}install

# save flags from Makefile.custom
LDFLAGS_PC:=${LDFLAGS} -L${LIB_INSTALL_PREFIX} -l${LIB} -Wl,-rpath=${LIB_INSTALL_PREFIX}
CXXFLAGS_PC:=$(filter-out -Isrc -Iapp -fPIC -Wall,${CXXFLAGS})
CXXCPP_PC:=${CXXCPP} -I${INC_INSTALL_PREFIX}

# parsing pkg-config files
ifneq "${PC_DEPS}" ""
CXXFLAGS:=${CXXFLAGS} $(shell pkg-config --cflags-only-I ${PC_DEPS})
CXXCPP:=${CXXCPP} $(shell pkg-config --cflags-only-other ${PC_DEPS})
#LDFLAGS:=$(shell pkg-config --libs-only-L ${PC_DEPS}) ${LDFLAGS} 
LDFLAGS:=$(patsubst -L%,${WL_RP}%,$(shell pkg-config --libs-only-L ${PC_DEPS})) ${LDFLAGS} 
LDFLAGS:=$(shell pkg-config --libs ${PC_DEPS}) ${LDFLAGS}
endif

LIB_LNK_INSTALL:=${LIB_LNK} -Wl,-rpath=${LIB_INSTALL_PREFIX}
LIB_LNK_LOCAL:=${LIB_LNK} -Wl,-rpath=${PWD}/lib

# checking for debug flag
ifeq "${DEBUG}" "TRUE"
CXXFLAGS:=${CXXFLAGS} -g -O0
DEBUG_INDICATOR=${FRED}"(debug)"${DEF}
else
CXXFLAGS:=${CXXFLAGS} -O4 -march=native -mtune=native -funroll-loops
DEBUG_INDICATOR=${FRED}"(release)"${DEF}
endif

# check for moc headers and search for moc command
ifneq "${MOC_HEADERS}" ""
MOC_OBJS=$(patsubst %.h,obj/%_moc.o,${MOC_HEADERS})
MOC_SOURCES=$(patsubst %.h,src/%_moc.cpp,${MOC_HEADERS})
MOC=$(shell pkg-config --variable=moc_location QtCore)
OBJS:=${MOC_OBJS} ${OBJS}
endif


ifeq "${SHOW_PROGRESS}" "TRUE"

ifndef PROGRESS
# works for up to 400 targets 
twenty := x x x x x x x x x x x x x x x x x x x x
MAX := $(foreach x,$(twenty),$(twenty))

# T estimates how many targets we are building by replacing BUILD with a special string
T := $(shell $(MAKE) NO_RECURSION=TRUE -nrRf $(firstword $(MAKEFILE_LIST)) $(MAKECMDGOALS) \
            PROGRESS="@echo COUNTTHIS > /dev/null" | grep -c "echo COUNTTHIS > /dev/null")

# N is the number of pending targets in base 1, well in fact, base x :-)
N := $(wordlist 1,$T,$(MAX)) 

# auto-decrementing counter that returns the number of pending targets in base 10
counter = $(words $N)$(eval N := $(wordlist 2,$(words $N),$N))

# BUILD is now defined to show the progress, this also avoids redefining T in loop
PROGRESS = ${INV}'['`echo "100-100*(${counter}-1)/(${T}+1)" | bc -l | cut -d '.' -f 1 | sed "s|^[0-9][0-9]$$|_&|g" | sed "s|^[0-9]$$|__&|g"`'%]'${DEF}
FIRST_RUN=FALSE
endif

endif

DIRS=lib obj bin src app app/obj
# actually we want to build doc/documentation
.PHONY: doc ${PC} test

all: ${DIRS} ${MOC_SOURCES} lib/${LIB_FILE} ${APPS} ${PKG_CFG} custom-build-hook

${DIRS}:
	${ECHO} ${PROGRESS} ${BYEL}[mkdir]${DEF} creating directory $@
	${AT}mkdir $@

test: test/runner
	./test/runner

test/runner: test/runner.cpp lib/${LIB_FILE} ${TEST_SOURCES}
	${ECHO} ${PROGRESS} ${BGRE}[tst]${DEF} building test
	${AT}${CXX} ./test/*.cpp ${CXXFLAGS} ${CXXCPP} ${LIB_LNK_LOCAL} ${LDFLAGS} -o $@ -lgtest -L${HOME}/local/lib -lpthread -I${HOME}/local/include

test/runner.cpp:
	${ECHO} ${PROGRESS} ${BGRE}[tst]${DEF} generating main test code file
	${AT}echo "#include <gtest/gtest.h>" > ./test/runner.cpp
	${AT}echo "int main(int argc, char **argv) {" >> ./test/runner.cpp
	${AT}echo "  ::testing::InitGoogleTest(&argc, argv);" >> ./test/runner.cpp
	${AT}echo "  return RUN_ALL_TESTS();" >> ./test/runner.cpp
	${AT}echo "}" >> ./test/runner.cpp

# overwrite build rules with these from dependency file
# if obj/dependencies is not found a submake is spawned
# to create it (make rock's)

ifeq "${FIRST_RUN}" "FALSE"
ifneq "${MAKECMDGOALS}" "cleaner"

-include obj/dependencies

endif
endif

ifneq "${SHOW_PROGRESS}" "TRUE"

-include obj/dependencies

endif


custom-install-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[install]${DEF} custom install

custom-build-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[build]${DEF} custom build

custom-clean-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[clean]${DEF} custom clean


obj/dependencies: ${DIRS} ${RECURSIVE_ICL_PROJECTS} ${DEP_FILES} 
	${AT}if [ "${DEP_FILES}" != "" ] ; then cat ${DEP_FILES} > $@ ; else touch $@ ; fi

src/%_moc.cpp: src/%.h
	${ECHO} ${PROGRESS} ${BBLU}[moc]${DEF} $@
	${AT}${MOC} $< -o $@

obj/%_moc.o: src/%_moc.cpp 
	${ECHO} ${PROGRESS} ${BCYA}[obj]${DEF} $@ ${DEBUG_INDICATOR}
	${AT}${CXX} -c ${CXXFLAGS} ${CXXCPP} $< -o $@


obj/%.o: src/%.cpp 
	${ECHO} ${PROGRESS} ${BCYA}[obj]${DEF} $@ ${DEBUG_INDICATOR}
	${AT}${CXX} -c ${CXXFLAGS} ${CXXCPP} $< -o $@

app/obj/%.o: app/%.cpp lib/${LIB_FILE}
	${ECHO} ${PROGRESS} ${BGRE}[obj]${DEF} $@
	${AT}${CXX} -c ${CXXFLAGS} ${CXXCPP} $< -o $@

bin/%: app/obj/%.o 
	${ECHO} ${PROGRESS} ${FBLA}${BGRE}[app]${DEF} $@
	${AT}${CXX} $< ${CXXFLAGS} ${CXXCPP} ${LIB_LNK_LOCAL} ${LDFLAGS} -o $@

lib/${LIB_FILE}: ${OBJS} 
	${ECHO} ${PROGRESS} ${BRED}[lib]${DEF} $@ ${FRED}${LIB_EMPTY_INDICATOR}${DEF}
	${AT}${LD} -shared ${OBJS} ${LDFLAGS}  -o $@

obj/%.src.dep: src/%.cpp 
	${ECHO} ${PROGRESS} ${BPUR}[dep]${DEF} $@
	${AT}${DEP} ${CXXFLAGS} ${CXXCPP} -MT `echo $< | sed "s|src/|obj/|g" | sed "s|.cpp|.o|g"` $< -MM -o ${@}_tmp && mv ${@}_tmp $@

obj/%.app.dep: app/%.cpp
	${ECHO} ${PROGRESS} ${BPUR}[dep]${DEF} $@
	${AT}${DEP} ${CXXFLAGS} ${CXXCPP} -MT `echo $< | sed "s|^app|app/obj|g" | sed "s|.cpp|.o|g"` $< -MM -o ${@}_tmp && mv ${@}_tmp $@

doc:
	${AT}(cd doc && doxygen 2>&1 | while read LINE ; do ${ECHO_NO_AT} ${BPUR}[doxygen]${DEF} $${LINE} ; done)

clean: custom-clean-hook
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} objects
	${AT}rm -rf obj/*.o app/obj/*.o
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} moc files
	${AT}rm -rf ${MOC_SOURCES}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} apps
	${AT}rm -rf ${APPS} 
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} ${LIB_FILE}
	${AT}rm -rf lib/${LIB_FILE}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} ${PC}
	${AT}rm -rf ${PC}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} ${TEST}
	${AT}rm -rf ${TEST}

cleaner: clean
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} temporary files
	${AT}rm -rf *~ src/*~ app/*~
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} dependency files
	${AT}rm -rf obj/*.dep obj/dependencies
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} documentation
	${AT}rm -rf doc/documentation


${PC}:
	${ECHO} ${PROGRESS} ${BYEL}[create]${DEF} $@
	${AT}rm -rf $@
	${ECHO} prefix=${INSTALL_PREFIX} >> $@
	${ECHO} exec_prefix=$$\{prefix\} >> $@
	${ECHO} bindir=$$\{prefix\}/${REL_BIN_INSTALL_PREFIX} >> $@
	${ECHO} libdir=$$\{prefix\}/lib >> $@
	${ECHO} datadir$$\{prefix\}/data >> $@
	${ECHO} includedir=$$\{prefix\}/${REL_INC_INSTALL_PREFIX} >> $@
	${ECHO} installation=${MAKECMDGOALS} >> $@
	${ECHO} package=${LIB} >> $@
	${ECHO} >> $@
	${ECHO} Name: ${LIB} >> $@
	${ECHO} Description: ${PACKAGE_DESCRIPTION} >> $@
	${ECHO} Version: ${VERSION} >> $@
	${ECHO} >> $@
	${ECHO} Requires: ${PC_DEPS} >> $@
	${ECHO} >> $@
	${ECHO} Libs: ${LDFLAGS_PC} >> $@
	${ECHO} >> $@
	${ECHO} Cflags: ${CXXFLAGS_PC} ${CXXCPP_PC} >> $@


create-install-dirs:
	${AT}for D in ${INSTALL_PREFIX} \
		      ${INC_INSTALL_PREFIX} \
		      ${DOC_INSTALL_PREFIX} \
		      ${ETC_INSTALL_PREFIX} \
		      ${LIB_INSTALL_PREFIX}/pkgconfig \
	              ${BIN_INSTALL_PREFIX} \
		      ${INSTALL_PREFIX}/doc \
		      ${INSTALL_PREFIX}/doc/html ; do \
			if [ ! -d $$D ] ; then \
				mkdir -m 775 -p $$D ; \
			fi ; \
	 done


installlink: all ${PC} uninstall create-install-dirs custom-install-hook
	${AT}for T in `find src/ -name *.h` ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} $$T ; \
		rm -rf ${INC_INSTALL_PREFIX}/$$T ; \
		ln -s ${PWD}/$$T ${INC_INSTALL_PREFIX}/ ; \
	done
	${AT}for T in ${APPS} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} $$T ; \
		rm -rf ${BIN_INSTALL_PREFIX}/$$T ; \
		ln -s ${PWD}/$$T ${BIN_INSTALL_PREFIX} ; 	\
	done
	${AT}for T in ${EXTRA_INSTALL_TO_BIN} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[extra link]${DEF} $$T ; \
		rm -rf ${BIN_INSTALL_PREFIX}/$$T ;\
		ln -s ${PWD}/$$T ${BIN_INSTALL_PREFIX} ; 	\
	done
	${ECHO} ${PROGRESS} ${BBLU}[link]${DEF} ${LIB_FILE}
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/${LIB_FILE}
	${AT}ln -s ${PWD}/lib/${LIB_FILE} ${LIB_INSTALL_PREFIX}
	${ECHO} ${PROGRESS} ${BBLU}[link]${DEF} ${PC} 
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}ln -s ${PWD}/${PC} ${LIB_INSTALL_PREFIX}/pkgconfig/
	${AT}if [ -d doc/documentation ] ; then \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} documentation ; \
		for T in doc/documentation/html/* ; do \
			rm -rf ${DOC_INSTALL_PREFIX}/$$T ; \
			ln -s ${PWD}/$$T ${DOC_INSTALL_PREFIX}/ ; \
		done ; \
	else \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[skip]${DEF} documentation ;	\
	fi
	${AT}for T in ${EXTRA_INSTALL_TO_ETC} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[extra link]${DEF} $$T ; \
		rm -rf ${ETC_INSTALL_PREFIX}/$$T ; \
		ln -s ${PWD}/$$T ${ETC_INSTALL_PREFIX} ; 	\
	done



${BIN_INSTALL_PREFIX}/%: app/obj/%.o
	${AT}rm -rf $@
	${ECHO} ${PROGRESS} ${BYEL}[install/relink]${DEF} $@
	${AT}${CXX} ${LIB_LNK_INSTALL} $< ${CXXFLAGS} ${CXXCPP}  ${LDFLAGS} -o $@
	${AT}chmod 775 $@



install: all ${PC} create-install-dirs custom-install-hook ${APPS_INSTALLED}
	${AT}for T in ${APPS_INSTALLED} ; do \
		if [ -L $$T ] ; then \
			echo "${BRED}[warning]${DEF} target $$T is a symbolic link" ; \
			echo "${BRED}[warning]${DEF} (forcing relink) this does only happen when you" ; \
			echo "${BRED}[warning]${DEF} switch from 'make installlink' to 'make install'" ; \
			rm -rf $$T ; \
			make $$T ; \
		fi \
	done
	${AT}for T in `find src/ -name *.h` ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[install]${DEF} $$T ; \
		rm -rf ${INC_INSTALL_PREFIX}/$$T ; \
		install -D -p -m 664 ${PWD}/$$T ${INC_INSTALL_PREFIX}/ ; \
	done
	${AT}for T in ${EXTRA_INSTALL_TO_BIN} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${FBLA}${BRED}[extra bin]${DEF} $$T ; \
		rm -rf ${INC_INSTALL_PREFIX}/$$T ; \
		cp $$T ${BIN_INSTALL_PREFIX}/ ; \
		chmod 775 ${BIN_INSTALL_PREFIX}/$$T ; \
	done
	${ECHO} ${PROGRESS} ${BYEL}[install]${DEF} ${LIB_FILE}
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/${LIB_FILE}
	${AT}install -D -p -m 664 ${PWD}/lib/${LIB_FILE} ${LIB_INSTALL_PREFIX}
	${ECHO} ${PROGRESS} ${BYEL}[install]${DEF} ${PC} 
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}install -D -p -m 664 ${PWD}/${PC} ${LIB_INSTALL_PREFIX}/pkgconfig/
	${AT}if [ -d doc/documentation ] ; then \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[install]${DEF} documentation ; \
		for T in doc/documentation/html/* ; do \
			rm -rf ${DOC_INSTALL_PREFIX}/$$T ; \
			install -D -p -m 664 ${PWD}/$$T ${DOC_INSTALL_PREFIX} ; \
		done ; \
	else \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[skip]${DEF} documentation ;	\
	fi
	${AT}for T in ${EXTRA_INSTALL_TO_ETC} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${FBLA}${BRED}[extra etc]${DEF} $$T ; \
		rm -rf ${ETC_INSTALL_PREFIX}/$$T : \
		install -p -m 664 $$T ${ETC_INSTALL_PREFIX}/ ; \
	done

uninstall:
	${ECHO} ${PROGRESS} ${BRED}[uninstall]${DEF} ${PFX}
	${AT}rm -rf ${INC_INSTALL_PREFIX}
	${AT}rm -rf ${BIN_INSTALL_PREFIX}
	${AT}rm -rf ${ETC_INSTALL_PREFIX}
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/${LIB_FILE}
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}rm -rf ${DOC_INSTALL_PREFIX}



$(call NOTIFY_PACKAGE,IPP,         )
$(call NOTIFY_PACKAGE,QT,          )
$(call NOTIFY_PACKAGE,LIBDC,       )
$(call NOTIFY_PACKAGE,UNICAP,      )
$(call NOTIFY_PACKAGE,LIBZ,        )
$(call NOTIFY_PACKAGE,LIBJPEG,     )
$(call NOTIFY_PACKAGE,SVS,         )
$(call NOTIFY_PACKAGE,IMAGEMAGICK, )
$(call NOTIFY_PACKAGE,XCF,         )

info:
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}${BBLI}ICL-Configuration${DEF}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}ICL${DEF}"         " `pkg-config --variable=prefix icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| IPP         " `pkg-config --variable=IPP_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| QT          " `pkg-config --variable=QT_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBDC       " `pkg-config --variable=LIBDC_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| UNICAP      " `pkg-config --variable=UNICAP_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBZ        " `pkg-config --variable=LIBZ_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBJPEG     " `pkg-config --variable=LIBJPEG_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| SVS         " `pkg-config --variable=SVS_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| IMAGEMAGICK " `pkg-config --variable=IMAGEMAGICK_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| XCF         " `pkg-config --variable=XCF_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| Flags"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}DEBUG${DEF}"       " ${DEBUG}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}LDFLAGS${DEF}"     " ${LDFLAGS}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}CXXFLAGS${DEF}"    " ${CXXFLAGS}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}CXXCPP${DEF}"      " ${CXXCPP}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"

update-from-svn:
	${ECHO} ${PROGRESS} ${BRED}[svn-update]${DEF} updating Makefile from svn at https://opensource.cit-ec.de/
	${AT}svn export https://opensource.cit-ec.de/svn/icl/trunk/scripts/icl-create-project-data/Makefile.template > /dev/null
	${ECHO} ${PROGRESS} ${BRED}[svn-update]${DEF} creating backup Makefile.old
	${AT}cp Makefile Makefile.old
	${AT}mv Makefile.template Makefile
	${ECHO} ${PROGRESS} ${BRED}[svn-update]${DEF} updating Makefile.custom from svn at https://opensource.cit-ec.de/
	${AT}svn export https://opensource.cit-ec.de/svn/icl/trunk/scripts/icl-create-project-data/Makefile.custom.template > /dev/null
	${ECHO} ${PROGRESS} ${BRED}[svn-update]${DEF} creating backup Makefile.custom.old
	${AT}cp Makefile.custom Makefile.custom.old
	${AT}mv Makefile.custom.template Makefile.custom
	${ECHO} ${PROGRESS} ${BGRE}[note:]${DEF} please transfer your custom changes from Makefile.custom.old to the new Makefile.custom
