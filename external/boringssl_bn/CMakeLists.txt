project (boringssl_bn)

include_directories(
    ${EXTERNAL_DIR}/boringssl_bn/inc)

SET(BORINGSSL_BN_SOURCE_FILES
  bcm.c
)

add_library (boringssl_bn ${BORINGSSL_BN_SOURCE_FILES})
