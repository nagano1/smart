set(BINARY ${CMAKE_PROJECT_NAME}_tst_long_running)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false
        ../../src/codenodes/*.cpp
        ../../src/thread_pool.cpp
        test.cpp
        test.h
        )

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})
if(DEFINED DOOR32)
    set_target_properties(${BINARY} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()


add_definitions("-DCMAKE_TEST=1")

if(NO_SPEED_TEST EQUAL 1)
    add_definitions("-DNO_SPEED_TEST=1")
else()
    #add_definitions("-DNO_SPEED_TEST=0")
endif()

#add_test(NAME ${BINARY} COMMAND ${BINARY})

find_package(Threads REQUIRED)
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib Threads::Threads)

