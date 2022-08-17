set(BINARY ${CMAKE_PROJECT_NAME}_lsp)


file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false
    ../src/codenodes/*.cpp
    ../src/utils.cpp
    ../src/LSP_server/*.cpp
    *.cpp
)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})
if(DEFINED DOOR32)
    set_target_properties(${BINARY} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()


#add_test(NAME ${BINARY} COMMAND ${BINARY})

find_package(Threads REQUIRED)
target_link_libraries(${BINARY} PUBLIC Threads::Threads)

