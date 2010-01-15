cmake_minimum_required (VERSION 2.6) 
project (SWIFT)

add_definitions ("-Wall -pedantic -fno-strict-aliasing -O2")
set (CMAKE_C_FLAGS "-std=c99")

#add_definitions(-DSWIFT_USE_CUSTOM_ALLOCATOR)
#add_definitions(-DSWIFT_USE_CUSTOM_THREAD_ALLOCATOR)
#add_definitions(-DLOGGING_ON)

if( CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX )
    add_definitions(-DGNU_COMPILER)
endif ()

add_subdirectory (src)
add_subdirectory (test)