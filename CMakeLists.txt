cmake_minimum_required(VERSION 3.25)
project("Path Tracer" VERSION 0.1.0)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch dependencies
include(FetchContent)
FetchContent_Declare(glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         master
)

FetchContent_Declare(stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         master
)

FetchContent_Declare(tinybvh
    GIT_REPOSITORY  https://github.com/jbikker/tinybvh.git
    GIT_TAG         main
)

FetchContent_Declare(tinyobjloader
    GIT_REPOSITORY  https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG         v2.0.0rc13
)

FetchContent_MakeAvailable(glm stb tinyobjloader)
FetchContent_Populate(tinybvh)

# Set up dependency interface libs
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

add_library(tinybvh INTERFACE)
target_include_directories(tinybvh INTERFACE "${tinybvh_SOURCE_DIR}")

# Set up project
file(GLOB_RECURSE PATH_TRACER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE PATH_TRACER_HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE PATH_TRACER_ASSETS CONFIGURE_DEPENDS "assets/*")

add_executable(PathTracer ${PATH_TRACER_SOURCES} ${PATH_TRACER_HEADERS})
target_link_libraries(PathTracer PRIVATE glm::glm stb tinybvh tinyobjloader)

if (NOT MSVC)
    target_compile_options(PathTracer PRIVATE -march=native)
endif()

# Copy assets on build
foreach(ASSET IN LISTS PATH_TRACER_ASSETS)
    get_filename_component(ASSET_PATH ${ASSET} ABSOLUTE)
    get_filename_component(ASSET_NAME ${ASSET} NAME)
    set(ASSET_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/assets/${ASSET_NAME}")

    add_custom_command(OUTPUT ${ASSET_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_PATH} ${ASSET_OUTPUT}
        COMMENT "Assets: generating ${ASSET_OUTPUT}"
        DEPENDS ${ASSET_PATH}
        VERBATIM
    )

    list(APPEND ASSET_OUTPUTS ${ASSET_OUTPUT})
endforeach()

add_custom_target(AssetCopy ALL DEPENDS ${ASSET_OUTPUTS})
add_dependencies(PathTracer AssetCopy)
