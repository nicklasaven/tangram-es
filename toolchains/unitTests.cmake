# options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -stdlib=libc++ -std=c++11")

add_definitions(-DPLATFORM_OSX) 

# include headers for homebrew-installed libraries
include_directories(/usr/local/include)

# load core library
include_directories(${PROJECT_SOURCE_DIR}/core/include/)
include_directories(${PROJECT_SOURCE_DIR}/core/include/jsoncpp/)
include_directories(${PROJECT_SOURCE_DIR}/core/dependencies/fontstash/fontstash/)
add_subdirectory(${PROJECT_SOURCE_DIR}/core)
include_recursive_dirs(${PROJECT_SOURCE_DIR}/core/src/*.h)

set(OSX_PLATFORM_SRC ${PROJECT_SOURCE_DIR}/osx/src/platform_osx.mm)

find_library(OPENGL_FRAMEWORK OpenGL)
find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
find_library(COCOA_FRAMEWORK Cocoa)

list(APPEND OSX_LIBRARIES 
    ${OPENGL_FRAMEWORK} 
    ${COCOA_FRAMEWORK}
    ${CORE_FOUNDATION_FRAMEWORK})

file(GLOB TEST_SOURCES tests/unit/*.cpp)

# create an executable per test
foreach(_src_file_path ${TEST_SOURCES})
    string(REPLACE ".cpp" "" test_case ${_src_file_path})
    string(REGEX MATCH "([^/]*)$" test_name ${test_case})
    
    set(EXECUTABLE_NAME "${test_name}.out")

    add_executable(${EXECUTABLE_NAME} ${_src_file_path} ${OSX_PLATFORM_SRC})

    target_link_libraries(${EXECUTABLE_NAME} -lcurl)
    target_link_libraries(${EXECUTABLE_NAME} core ${OSX_LIBRARIES})
endforeach(_src_file_path ${TEST_SOURCES})

# copy resources in order to make tests with resources dependency
file(GLOB_RECURSE RESOURCES ${PROJECT_SOURCE_DIR}/core/resources/*)
foreach(_resource ${RESOURCES})
    file(COPY ${_resource} DESTINATION ${PROJECT_SOURCE_DIR}/build/tests/unit/bin)
endforeach()
