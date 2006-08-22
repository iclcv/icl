#
#   makefile
#
#   Copyright (C) 2002 by
#   Honda R&D EUROPE (DEUTSCHLAND) GmbH,
#   Future Technology Research
#   Carl-Legien Str. 30
#   63073 Offenbach/Main
#   Germany
#
#   UNPUBLISHED PROPRIETARY MATERIAL.
#   ALL RIGHTS RESERVED.
#
#   Author: Antonello Ceravola & Mark Dunn & Julian Eggert
#
#   Date: September 13, 2002
#

# Don't change anything below this line!
###############################################################################

# Do not assign anything to this variables
_DOT_DOT=

ifeq "${_CURR_DIR}" ""
 _CURR_DIR=.
else
 _CURR_DIR+=
endif

PROJECT_NAME:= $(notdir $(shell pwd))

include makeVar

_DIRECTORY_LIST:=$(filter-out ${SKIP_DIRECTORIES},$(subst /makefile,,$(wildcard */makefile)))
_DIRECTORY_ORDER:=$(filter ${_DIRECTORY_LIST},${DIR_PRIORITY}) \
                  $(filter-out ${DIR_PRIORITY} ${DIR_LOWPRIORITY},${_DIRECTORY_LIST}) \
                  $(filter ${_DIRECTORY_LIST},${DIR_LOWPRIORITY})

#-----------------------------------------------
# Rule: all & cleanall & clean & printtype
#-----------------------------------------------
all: ${ALL_EXEC_LIST}

cleanall: $(ALL_EXEC_LIST:%=clean%)

clean: $(_DIRECTORY_ORDER:%=clean_%)

clean_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} clean -C $(subst clean_,,$@)\
              "_CURR_DIR=$(subst clean_,${strip ${_CURR_DIR}}/,$@)"
	${PRE_CMD}${CMD_RM} *~ *.bak

printtype:
	${PRE_CMD}${CMD_ECHO} "Type: RecursiveDescend"

#-----------------------------------------------
# Rule: doc & cleandoc
#-----------------------------------------------
doc: $(_DIRECTORY_ORDER:%=doc_%)

doc_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} doc -C $(subst doc_,,$@)\
              "_CURR_DIR=$(subst doc_,${strip ${_CURR_DIR}}/,$@)"

cleandoc: $(_DIRECTORY_ORDER:%=cleandoc_%)

cleandoc_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} cleandoc -C $(subst cleandoc_,,$@)\
              "_CURR_DIR=$(subst cleandoc_,${strip ${_CURR_DIR}}/,$@)"
	${PRE_CMD}${CMD_RM} *~ *.bak

#-----------------------------------------------
# Rule: obj & cleanobj
#-----------------------------------------------
obj: $(_DIRECTORY_ORDER:%=obj_%)

obj_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} obj -C $(subst obj_,,$@)\
              "_CURR_DIR=$(subst obj_,${strip ${_CURR_DIR}}/,$@)"

cleanobj: $(_DIRECTORY_ORDER:%=cleanobj_%)

cleanobj_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} cleanobj -C $(subst cleanobj_,,$@)\
              "_CURR_DIR=$(subst cleanobj_,${strip ${_CURR_DIR}}/,$@)"
	${PRE_CMD}${CMD_RM} *~ *.bak

#-----------------------------------------------
# Rule: lib & cleanlib
#-----------------------------------------------
lib: $(_DIRECTORY_ORDER:%=lib_%)

lib_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} lib -C $(subst lib_,,$@)\
              "_CURR_DIR=$(subst lib_,${strip ${_CURR_DIR}}/,$@)"

cleanlib: $(_DIRECTORY_ORDER:%=cleanlib_%)

cleanlib_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} cleanlib -C $(subst cleanlib_,,$@)\
              "_CURR_DIR=$(subst cleanlib_,${strip ${_CURR_DIR}}/,$@)"
	${PRE_CMD}${CMD_RM} *~ *.bak

#-----------------------------------------------
# Rule: exe & cleanexe
#-----------------------------------------------
exe: $(_DIRECTORY_ORDER:%=exe_%)

exe_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} exe -C $(subst exe_,,$@)\
              "_CURR_DIR=$(subst exe_,${strip ${_CURR_DIR}}/,$@)"

cleanexe: $(_DIRECTORY_ORDER:%=cleanexe_%)

cleanexe_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} cleanexe -C $(subst cleanexe_,,$@)\
              "_CURR_DIR=$(subst cleanexe_,${strip ${_CURR_DIR}}/,$@)"
	${PRE_CMD}${CMD_RM} *~ *.bak

#-----------------------------------------------
# Rule: install & cleaninstall
#-----------------------------------------------
install: $(_DIRECTORY_ORDER:%=install_%)

install_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} install -C $(subst install_,,$@)\
              "_CURR_DIR=$(subst install_,${strip ${_CURR_DIR}}/,$@)"

cleaninstall: $(_DIRECTORY_ORDER:%=cleaninstall_%)

cleaninstall_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} cleaninstall -C $(subst cleaninstall_,,$@)\
              "_CURR_DIR=$(subst cleaninstall_,${strip ${_CURR_DIR}}/,$@)"

#-----------------------------------------------
# Rule: depend
#-----------------------------------------------
depend: $(_DIRECTORY_ORDER:%=depend_%)

depend_%:
	${PRE_CMD}${CMD_MAKE} ${EXTRAMAKEFLAGS} depend -C $(subst depend_,,$@)\
              "_CURR_DIR=$(subst depend_,${strip ${_CURR_DIR}}/,$@)"











