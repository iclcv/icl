icl_check_external_package(RSB "rsb/Factory.h;rsb/Handler.h;rsb/converter/Repository.h;rsb/converter/ProtocolBufferConverter.h" "rsbcore" lib include TRUE TRUE)


icl_check_external_package(PROTOBUF "google/protobuf/stubs/common.h;google/protobuf/generated_message_util.h;google/protobuf/repeated_field.h;google/protobuf/extension_set.h;google/protobuf/generated_message_reflection.h;google/protobuf/stubs/once.h;google/protobuf/io/coded_stream.h;google/protobuf/wire_format_lite_inl.h;google/protobuf/descriptor.h;google/protobuf/reflection_ops.h;google/protobuf/wire_format.h" "protobuf" lib include FALSE FALSE)

find_program(PROTOC_CMD protoc HINTS "${ICL_XDEP_PROTOBUF_PATH}/bin" DOC "location of the protocol buffer compiler")
if(NOT "${PROTOC_CMD}" STREQUAL "PROTOC_CMD-NOTFOUND") 
  message(STATUS "found (binary): protoc")
else()
  message(STATUS "not found (binary): protoc")
  set(HAVE_PROTOBUF_COND FALSE)
endif()

if(HAVE_PROTOBUF_COND)
  message(STATIS "PROTOBUF detected: TRUE")
else()
  message(STATIS "PROTOBUF detected: FALSE")
endif()



