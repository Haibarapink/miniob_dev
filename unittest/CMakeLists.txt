# 可以获取父cmake的变量
MESSAGE("${CMAKE_COMMON_FLAGS}")

find_package(GTest CONFIG REQUIRED)

include(GoogleTest)

ADD_SUBDIRECTORY(common)
IF (WITH_MEMTRACER)
    ADD_SUBDIRECTORY(memtracer)
ENDIF (WITH_MEMTRACER)

ADD_SUBDIRECTORY(observer)
ADD_SUBDIRECTORY(oblsm)

# 指定要复制的 JSON 文件
set(JSON_FILES
    "${CMAKE_SOURCE_DIR}/unittest/observer/radix/test/data.json"
    "${CMAKE_SOURCE_DIR}/unittest/observer/radix/test/expect.json"
    "${CMAKE_SOURCE_DIR}/unittest/observer/radix/test/operation.json"
)
# 创建目标目录
set(DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/radix")
file(MAKE_DIRECTORY ${DEST_DIR})

# 复制 JSON 文件到目标目录，并打印文件名
foreach(JSON_FILE ${JSON_FILES})
    get_filename_component(FILENAME ${JSON_FILE} NAME)
    if(EXISTS ${JSON_FILE})
        configure_file(${JSON_FILE} ${DEST_DIR}/${FILENAME} COPYONLY)
        message(STATUS "Copied file: ${FILENAME} to ${DEST_DIR}")
    else()
        message(WARNING "File ${JSON_FILE} does not exist. Skipping copy operation.")
    endif()
endforeach()    