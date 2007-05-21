# -*-mode: Makefile;-*-  ;; Have EMACS always use makefile-mode for this file. 
#
#   makefile
#

# Don't change anything in this file!
###############################################################################

# Do not assign anything to this variable
_DOT_DOT=

ifeq "${_CURR_DIR}" ""
 _CURR_DIR=.
endif

include ../makeVar

#-----------------------------------------------
# Rule: all
#-----------------------------------------------
all: install

#-----------------------------------------------
# Rule: installProj
#-----------------------------------------------
install:
	echo "**** Installing Project: ${PROJECT_ROOT_NAME} ****"
	perl ./Install ${PROJECT_ROOT_NAME} 1

installdoc:
	echo "**** Installing Project documentation for: ${PROJECT_ROOT_NAME} ****"
	perl ./Install ${PROJECT_ROOT_NAME} 2

installlink:
	echo "**** Installing Project: ${PROJECT_ROOT_NAME} ****"
	perl ./Install ${PROJECT_ROOT_NAME} 3

installall:
	echo "**** Installing Project documentation for: ${PROJECT_ROOT_NAME} ****"
	perl ./Install ${PROJECT_ROOT_NAME} 0

#-----------------------------------------------
# Rule: % (default rule)
# Note: This rule must be the last one!
#-----------------------------------------------
%: ;
































