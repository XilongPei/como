project (EasyLogger)

SET(EASYLOGGER_SOURCE_FILES
    ../easylogger/src/elog_async.c
    ../easylogger/src/elog_buf.c
    ../easylogger/src/elog.c
    ../easylogger/src/elog_utils.c
    ../easylogger/plugins/file/elog_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/easylogger/port/elog_port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/easylogger/port/elog_file_port.c
)

SET(EASYLOGGER_INCLUDE_FILES
    ../easylogger/inc/elog_cfg.h
    ../easylogger/inc/elog.h
    ../easylogger/plugins/file/elog_file_cfg.h
    ../easylogger/plugins/file/elog_file.h
    easylogger/inc/elog_cfg.h
    easylogger/inc/elog_file_cfg.h
)

include_directories(
    ./
    ${CMAKE_CURRENT_SOURCE_DIR}/easylogger/inc
    ../easylogger/inc
    ../easylogger/plugins/file
)

add_library(EasyLogger ${EASYLOGGER_SOURCE_FILES} ${EASYLOGGER_INCLUDE_FILES})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to EasyLogger
target_include_directories (EasyLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
