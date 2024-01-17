project (LibCRC)

SET(LibCRC_SOURCE_FILES
    ../src/crc8.c
    ../src/crc16.c
    ../src/crc32.c
    ../src/crc64.c
    ../src/crcccitt.c
    ../src/crcdnp.c
    ../src/crckrmit.c
    ../src/crcsick.c
    ../src/nmea-chk.c
)

SET(LibCRC_INCLUDE_FILES
    ../include/checksum.h
)

include_directories(
    ../include
)

add_library(LibCRC ${LibCRC_SOURCE_FILES} ${LibCRC_INCLUDE_FILES})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to LibCRC
target_include_directories(LibCRC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
