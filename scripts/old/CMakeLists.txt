set(P ${CMAKE_INSTALL_PREFIX})
set(E icl-${VERSION}${DEBUG_POSTFIX})
set(S "_")
set(BIN ${P}/bin)
set(ETC ${P}/etc)
set(LIB ${P}/lib)
set(DOC ${P}/doc)
set(INC ${P}/include)

set(PERMISSIONS "775")
install(SCRIPT CODE "message(STATUS calling${S}chmod${S}-f${S}${PERMISSIONS}${S}${P})")
install(SCRIPT CODE "execute_process(COMMAND chmod -f ${PERMISSIONS} ${P} RES OUT)")

set(X BIN ETC LIB DOC INC)
foreach(x ${X})
  install(SCRIPT CODE "message(STATUS calling${S}chmod${S}-f${S}${PERMISSIONS}${S}${${x}})")
  install(SCRIPT CODE "execute_process(COMMAND chmod -f ${PERMISSIONS} ${${x}} RES OUT)")
  install(SCRIPT CODE "message(STATUS calling${S}chmod${S}-R${S}-f${S}${PERMISSIONS}${S}${${x}}/${E})")
  install(SCRIPT CODE "execute_process(COMMAND chmod -R -f ${PERMISSIONS} ${${x}}/${E} RES OUT)")
endforeach()

#this does not work for some reasons ...
#install(SCRIPT CODE "message(STATUS you${S}can${S}fix${S}installed${S}documentation_references_manually)")
#install(SCRIPT CODE "execute_process(COMMAND echo call find ${DOC}/${E} -iname '*.html' | xargs sed -i 's|/home/celbrech/projects/ICL/doc/style\\\\.css|style\\\\.css|g' RES OUT)")
