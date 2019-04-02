cmake_minimum_required(VERSION 2.8.4)

project(VulkanTest)
set(PROJECT_VERSION "1.0")
set(FULL_PROJECT_NAME "Vulkan Test")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(APP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

file(GLOB_RECURSE APP_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
include_directories("${APP_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DFULL_PROJECT_NAME="${FULL_PROJECT_NAME}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 not found.")
endif()

if(NOT X11_Xrandr_FOUND)
    message(FATAL_ERROR "XRANDR not found.")
endif()

find_package(Vulkan REQUIRED)
include_directories(${VULKAN_INCLUDE_DIR})

if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan not found.")
endif()

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

if(NOT PNG_FOUND)
    message(FATAL_ERROR "PNG not found.")
endif()

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${X11_X11_LIB} 
                      ${X11_Xrandr_LIB} 
                      ${VULKAN_LIBRARY} 
                      ${PNG_LIBRARIES})
