SUBDIRS = @ICL_BUILD_SUBDIRS@
DOCDIRS = @ICL_BUILD_DOCDIRS@
ACLOCAL_AMFLAGS = -I m4
PACKAGES = @ICL_BUILD_PACKAGES@

EXTRA_DIST=bootstrap.sh @ICL_DOC_MAKEFILES_DIST@ doxyfile doc scripts ICLQt/examples/calib-radial-distortion-tools.h VERSION.sh 
MKDIR_P ?= mkdir -p

.PHONY: doc

doc:
	@echo "creating documentation recursively..."
	@for S in ${DOCDIRS} ; do ${MAKE} ${AM_FLAGS} -C$$S doc ; done

cleandoc:
	@echo "cleaning documentation recursively..."
	@for S in ${DOCDIRS} ; do ${MAKE} ${AM_FLAGS} -C$$S cleandoc ; done

install-data-local:
	@for S in ${PACKAGES} ; do ${MAKE} ${AM_FLAGS} -C$$S install-data-local ; done
	@scripts/create-doc-menu.sh ${PACKAGES}
	${MKDIR_P} ${prefix}/doc
	${MKDIR_P} ${prefix}/doc/ICL
	${INSTALL_DATA} doc/*.html ${prefix}/doc/ICL
	${INSTALL_DATA} doc/logo.jpg ${prefix}/doc/ICL
	${MKDIR_P} ${prefix}/lib
	${MKDIR_P} ${prefix}/lib/pkgconfig
	if [ -e icl.pc ] ; then ${INSTALL_DATA} icl.pc ${prefix}/lib/pkgconfig ; fi
	${INSTALL_PROGRAM} scripts/icl-make ${prefix}/bin


uninstall-local:
	@for S in ${PACKAGES} ; do ${MAKE} ${AM_FLAGS} -C$$S uninstall-local ; done
	@rm -rf ${prefix}/doc/ICL
	@rm -rf ${prefix}/lib/pkgconfig
	@rm -rf ${prefix}/bin/icl-make

