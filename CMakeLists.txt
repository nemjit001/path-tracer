cmake_minimum_required(VERSION 3.25)
project("Path Tracer" VERSION 0.1.0)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         master
)

FetchContent_Declare(stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         master
)

FetchContent_Declare(tinyobjloader
    GIT_REPOSITORY  https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG         v2.0.0rc13
)

FetchContent_MakeAvailable(glm stb tinyobjloader)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

file(GLOB_RECURSE PATH_TRACER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE PATH_TRACER_HEADERS CONFIGURE_DEPENDS "src/*.hpp")

add_executable(PathTracer ${PATH_TRACER_SOURCES} ${PATH_TRACER_HEADERS})
target_link_libraries(PathTracer PRIVATE glm::glm stb tinyobjloader)
