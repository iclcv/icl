SUBDIRS = @ICL_BUILD_SUBDIRS@
DOCDIRS = @ICL_BUILD_DOCDIRS@
ACLOCAL_AMFLAGS = -I m4
PACKAGES = @ICL_BUILD_PACKAGES@

EXTRA_DIST=bootstrap.sh @ICL_DOC_MAKEFILES_DIST@ doxyfile doc scripts ICLQt/examples/calib-radial-distortion-tools.h VERSION.sh 
MKDIR_P ?= mkdir -p


.PHONY: doc alldoc

doc:
	@echo "creating documentation recursively..."
	@for S in ${DOCDIRS} ; do ${MAKE} ${AM_FLAGS} -C$$S doc ; done

alldoc:
	@echo "creating combined documentation for all packages"
	@(cd alldoc ; doxygen doxyfile.all ; cp ../doc/tab_*.gif html/)

cleandoc:
	@echo "cleaning documentation recursively..."
	@for S in ${DOCDIRS} ; do ${MAKE} ${AM_FLAGS} -C$$S cleandoc ; done

install-data-local:
	@for S in ${PACKAGES} ; do ${MAKE} ${AM_FLAGS} -C$$S install-data-local ; done
	@if [ -d alldoc/html ] ; then \
		${MKDIR_P} ${prefix}/doc && \
		${MKDIR_P} ${prefix}/doc/ALL && \
		cp -a alldoc/html/* ${prefix}/doc/ALL/ && \
		cp -a alldoc/*.html ${prefix}/doc/ALL/ && \
		chmod -R g+rwX ${prefix}/doc/ALL ; \
	else \
		echo "combined documentation for all packages not found (skipping)" ;\
	fi
	@scripts/create-doc-menu.sh ${PACKAGES}
	${MKDIR_P} ${prefix}/doc
	${MKDIR_P} ${prefix}/doc/ICL
	${INSTALL_DATA} doc/*.html ${prefix}/doc/ICL
	${INSTALL_DATA} doc/menuStyle.css ${prefix}/doc/ICL/style.css
	${INSTALL_DATA} doc/logo.png ${prefix}/doc/ICL
	${MKDIR_P} ${prefix}/lib
	${MKDIR_P} ${prefix}/lib/pkgconfig
	@for T in ${PACKAGES} icl ; do \
		if [ -e $${T}.pc ] ; then \
			${INSTALL_DATA} $${T}.pc ${prefix}/lib/pkgconfig ; \
			echo installing $${T}.pc to ${prefix}/lib/pkgconfig/$${T}.pc ; \
		else \
			echo ERROR: $${T}.pc is missing ; \
		fi \
	done
	${INSTALL_PROGRAM} scripts/icl-make ${prefix}/bin
	${INSTALL_PROGRAM} scripts/icl-create-project ${prefix}/bin
	sed -i "s|RESOURCE_DIR_PLACEHOLDER|${prefix}/etc|g" ${prefix}/bin/icl-create-project
	${MKDIR_P} ${prefix}/etc
	${INSTALL_DATA} scripts/icl-create-project-data/*.template ${prefix}/etc

uninstall-local:
	@for S in ${PACKAGES} ; do ${MAKE} ${AM_FLAGS} -C$$S uninstall-local ; done
	@rm -rf ${prefix}/doc/ICL
	@rm -rf ${prefix}/lib/pkgconfig
	@rm -rf ${prefix}/bin/icl-make
	@rm -rf ${prefix}/bin/icl-create-project
	@rm -rf ${prefix}/etc/Makefile.template
	@rm -rf ${prefix}/etc/Makefile.custom.template
	@rm -rf ${prefix}/etc/doxyfile.template

