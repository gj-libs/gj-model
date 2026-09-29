cmake_minimum_required(VERSION 3.23)

project(gj-model LANGUAGES C)

option(GJ_MODEL_BUILD_TESTS "Build gj-model tests" OFF)

add_subdirectory(deps/gj-json)

add_library(gj-model STATIC)

target_sources(gj-model
    PRIVATE
        src/core/model.c
        src/formats/mtl.c
        src/formats/obj.c
        src/formats/stl.c

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/gj_model/gj_model.h
)

target_include_directories(gj-model
    PRIVATE
        src
    PUBLIC
        include
)

target_link_libraries(gj-model
    PRIVATE
        gj-json
)

if(GJ_MODEL_BUILD_TESTS)
    add_executable(gj-model-test
        src/test/test.c
    )

    target_include_directories(gj-model-test
        PRIVATE
            src
    )

    target_link_libraries(gj-model-test
        PRIVATE
            gj-model
    )
endif()
