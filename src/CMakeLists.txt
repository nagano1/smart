set(BINARY ${CMAKE_PROJECT_NAME})

#file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

#set(SOURCES ${SOURCES})

file(GLOB cpp_files
    "./main.cpp"

    #"./codenodes/*.cpp"
)



add_executable(${BINARY}_run ${cpp_files})
if(DEFINED DOOR32)
    set_target_properties(${BINARY}_run PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()



install(TARGETS ${BINARY}_run
        RUNTIME	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		BUNDLE	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
 		LIBRARY	DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


add_library(${BINARY}_lib STATIC ${cpp_files})
if(DEFINED DOOR32)
    set_target_properties(${BINARY}_lib PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

find_package(Threads REQUIRED)
target_link_libraries(${BINARY}_run Threads::Threads)


