cmake_minimum_required(VERSION 2.8.12)
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
include(${ROOT_DIR}/contrib/common.cmake)

# Additional applications that can be used to test the library

option(PSMOVE_BUILD_EXAMPLES "Build the C examples" ON)
option(PSMOVE_BUILD_OPENGL_EXAMPLES "Build the OpenGL examples" OFF)
option(PSMOVE_BUILD_TESTS "Build the C tests" ON)
option(PSMOVE_BUILD_TUIO_SERVER "Build the TUIO server" OFF)

# C examples
if(PSMOVE_BUILD_EXAMPLES)
    foreach(EXAMPLE example multiple dump_calibration battery_check)
        add_executable(${EXAMPLE} ${CMAKE_CURRENT_LIST_DIR}/c/${EXAMPLE}.c)
	if(NOT WIN32)
	    target_link_libraries(${EXAMPLE} rt)
	endif(NOT WIN32)
        target_link_libraries(${EXAMPLE} psmoveapi)
    endforeach()

    if(PSMOVE_BUILD_TRACKER AND PSMOVE_BUILD_TUIO_SERVER)
        include_directories(${ROOT_DIR}/external/TUIO_CPP/TUIO)
        include_directories(${ROOT_DIR}/external/TUIO_CPP/oscpack)
        add_executable(tuio_server ${CMAKE_CURRENT_LIST_DIR}/c/tuio_server.cpp
            external/TUIO_CPP/TUIO/TuioClient.cpp
            external/TUIO_CPP/TUIO/TuioServer.cpp
            external/TUIO_CPP/TUIO/TuioTime.cpp
            external/TUIO_CPP/oscpack/osc/OscTypes.cpp
            external/TUIO_CPP/oscpack/osc/OscOutboundPacketStream.cpp
            external/TUIO_CPP/oscpack/osc/OscReceivedElements.cpp
            external/TUIO_CPP/oscpack/osc/OscPrintReceivedElements.cpp
            external/TUIO_CPP/oscpack/ip/posix/NetworkingUtils.cpp
            external/TUIO_CPP/oscpack/ip/posix/UdpSocket.cpp
            external/TUIO_CPP/oscpack/ip/win32/NetworkingUtils.cpp
            external/TUIO_CPP/oscpack/ip/win32/UdpSocket.cpp)
        set_target_properties(tuio_server PROPERTIES
            COMPILE_FLAGS -DOSC_HOST_LITTLE_ENDIAN)
        target_link_libraries(tuio_server psmoveapi psmoveapi_tracker)
    else()
        # Disable the TUIO Server if we don't build the tracker
        set(PSMOVE_BUILD_TUIO_SERVER OFF)
    endif()

    if(PSMOVE_BUILD_TRACKER)
        foreach(EXAMPLE distance_calibration)
            add_executable(${EXAMPLE} ${CMAKE_CURRENT_LIST_DIR}/c/${EXAMPLE}.c)
            target_link_libraries(${EXAMPLE} psmoveapi psmoveapi_tracker)
        endforeach()
        
        FIND_PACKAGE(OCULUS_SDK)
        IF(OCULUS_SDK_FOUND)
            add_executable(coregister_dk2 ${CMAKE_CURRENT_LIST_DIR}/c/coregister_dk2.cpp)
            include_directories(${OCULUS_SDK_INCLUDE_DIRS})
            target_link_libraries(coregister_dk2 psmoveapi psmoveapi_tracker ${OCULUS_SDK_LIBRARIES})
        ENDIF(OCULUS_SDK_FOUND)        
    endif()
endif()

# C test programs
if(PSMOVE_BUILD_TESTS)
    foreach(TESTNAME led_update read_performance responsiveness extension led_pwm_frequency gyroscope_error)
        add_executable(test_${TESTNAME} ${CMAKE_CURRENT_LIST_DIR}/c/test_${TESTNAME}.c)
	if(NOT WIN32)
	    target_link_libraries(test_${TESTNAME} rt)
	endif(NOT WIN32)
        target_link_libraries(test_${TESTNAME} psmoveapi)
    endforeach(TESTNAME)

    foreach(TESTNAME calibration)
        add_executable(test_${TESTNAME} ${CMAKE_CURRENT_LIST_DIR}/c/test_${TESTNAME}.cpp)
	if(NOT WIN32)
	    target_link_libraries(test_${TESTNAME} rt)
	endif(NOT WIN32)
        target_link_libraries(test_${TESTNAME} psmoveapi)
    endforeach(TESTNAME)

    if(PSMOVE_BUILD_TRACKER)
        foreach(TESTNAME tracker capture_performance record_video roi_size end2end_latency fusion)
            add_executable(test_${TESTNAME} ${CMAKE_CURRENT_LIST_DIR}/c/test_${TESTNAME}.c)
            target_link_libraries(test_${TESTNAME} psmoveapi psmoveapi_tracker)
        endforeach(TESTNAME)

        if (PSMOVE_BUILD_OPENGL_EXAMPLES)
            set(OPENGL_EXAMPLES_LIBRARIES "")

            IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
                find_library(OPENGL_FRAMEWORK OpenGL)
                find_library(GLUT_FRAMEWORK GLUT)
                FIND_PACKAGE(SDL2)
                include_directories(${SDL2_INCLUDE_DIR})
                list(APPEND OPENGL_EXAMPLES_LIBRARIES
                    ${SDL2_LIBRARY} ${OPENGL_FRAMEWORK} ${GLUT_FRAMEWORK})
            ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
                list(APPEND OPENGL_EXAMPLES_LIBRARIES SDL GL glut)
            ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
                SET(ENV{SDL2DIR} ${ROOT_DIR}/external/SDL2/)
                FIND_PACKAGE(SDL2)
                include_directories(${SDL2_INCLUDE_DIR})
                list(APPEND OPENGL_EXAMPLES_LIBRARIES ${SDL2_LIBRARY})
                list(GET SDL2_LIBRARY 0 FIRST_SDL2_LIB)
                get_filename_component(SDL_BIN_PATH ${FIRST_SDL2_LIB} DIRECTORY)
            ENDIF()

            foreach(EXAMPLE test_opengl test_opengl2 test_opengl3)
                add_executable(${EXAMPLE} ${CMAKE_CURRENT_LIST_DIR}/c/${EXAMPLE}.cpp)
                target_link_libraries(${EXAMPLE}
                    psmoveapi
                    psmoveapi_tracker
                    ${OPENGL_EXAMPLES_LIBRARIES})
                IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
                    add_custom_command(TARGET ${EXAMPLE} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${SDL_BIN_PATH}/SDL2.dll"
                            $<TARGET_FILE_DIR:${EXAMPLE}>)
                ENDIF()
            endforeach()
        endif()
    endif()
endif()

message("")
message("  Additional targets")
feature_use_info("C example apps:   " PSMOVE_BUILD_EXAMPLES)
feature_use_info("OpenGL examples:  " PSMOVE_BUILD_OPENGL_EXAMPLES)
feature_use_info("C test programs:  " PSMOVE_BUILD_TESTS)
feature_use_info("C++ TUIO server:  " PSMOVE_BUILD_TUIO_SERVER)
message("")
