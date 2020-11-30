# ----------------------------------------------------------------------------
#   Basic Configuration
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)

project(RekkonMMALCamera LANGUAGES CXX)

set(PROJECT_VERSION "0.1.0")
string(REGEX MATCHALL "[0-9]" PROJECT_VERSION_PARTS "${PROJECT_VERSION}")
list(GET PROJECT_VERSION_PARTS 0 PROJECT_VERSION_MAJOR)
list(GET PROJECT_VERSION_PARTS 1 PROJECT_VERSION_MINOR)
list(GET PROJECT_VERSION_PARTS 2 PROJECT_VERSION_PATCH)
set(PROJECT_SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------------------------------------------
# Build type
#------------------------------------------------------

IF(NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE "Release" )
ENDIF()


OPTION(BUILD_SHARED_LIBS 	"Set to OFF to build static libraries" ON)

# ----------------------------------------------------------------------------
#   Uninstall target, for "make uninstall"
# ----------------------------------------------------------------------------
CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# ----------------------------------------------------------------------------
# create configuration file from .in file (If you use windows take care with paths)
# ----------------------------------------------------------------------------

CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/config.cmake.in" "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" DESTINATION share/${PROJECT_NAME} )


IF(NOT TARGET_PROCESSOR )
    SET(TARGET_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()


if(${TARGET_PROCESSOR} MATCHES armv7l) # In ARM_COrtex8 with neon, enalble vectorized operations
    set(GENERAL_FLAGS "${GENERAL_FLAGS} -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard ")
endif()
if(${TARGET_PROCESSOR} MATCHES armv6l) # In PiZero with neon, enalble vectorized operations
    set(GENERAL_FLAGS "${GENERAL_FLAGS}  -mabi=aapcs-linux -marm  -march=armv6 -mfloat-abi=hard  -mfp16-format=none -mfpu=vfp -mlittle-endian -mpic-data-is-text-relative -mrestrict-it -msched-prolog -mstructure-size-boundary=0x20 -mtp=auto -mtls-dialect=gnu -munaligned-access -mvectorize-with-neon-quad")
endif()


SET(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS} ${GENERAL_FLAGS}  -O3 -g0  -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS} ${GENERAL_FLAGS}  -O0 -g3  -DDEBUG -D_DEBUG -DPRINT_DEBUG_MESSAGES")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS} ${GENERAL_FLAGS}  -O1 -g3  -D_DEBUG -DDEBUG -DPRINT_DEBUG_MESSAGES")


SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${EXTRA_EXE_LINKER_FLAGS_RELEASE}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${EXTRA_EXE_LINKER_FLAGS_DEBUG}")



# ----------------------------------------------------------------------------
# Create module
# ----------------------------------------------------------------------------

SET(REQUIRED_LIBRARIES ${REQUIRED_LIBRARIES} pthread)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)

FIND_LIBRARY(mmalcore_LIBS NAMES mmal_core PATHS /opt/vc/lib)
FIND_LIBRARY(mmalutil_LIBS NAMES mmal_util PATHS /opt/vc/lib)
FIND_LIBRARY(mmal_LIBS NAMES mmal PATHS /opt/vc/lib)
FIND_LIBRARY(bcm_host_LIBS NAMES bcm_host PATHS /opt/vc/lib)
#IF( (NOT mmal_LIBS ) OR (NOT mmalutil_LIBS) OR (NOT mmalcore_LIBS) OR (NOT bcm_host_LIBS))
 IF( (NOT mmal_LIBS ) OR (NOT mmalutil_LIBS) OR (NOT mmalcore_LIBS))
    MESSAGE(FATAL_ERROR "Could not find mmal libraries")
 ENDIF()
 #set(REQUIRED_LIBRARIES ${REQUIRED_LIBRARIES} ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS} ${bcm_host_LIBS} )
 set(REQUIRED_LIBRARIES ${REQUIRED_LIBRARIES} ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS})
 ELSE()
 SET(srcs_base ${srcs_base} dependencies/fake_mmal_dependencies.cpp)
ENDIF()
 include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/interface/vcos" "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/interface" "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

INCLUDE_DIRECTORIES(.)


SET(public_hdrs_base rekkoncamcontrol.h videommalobject.h)
SET(hdrs_base ${public_hdrs_base} )
SET(srcs_base ${srcs_base} rekkoncamcontrol.cpp videommalobject.cpp)

add_library(RekkonMMALCamera
    ${hdrs_base}
    ${srcs_base}
)

SET_TARGET_PROPERTIES(RekkonMMALCamera PROPERTIES 		# create *nix style library versions + symbolic links
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_SOVERSION}
        CLEAN_DIRECT_OUTPUT 1 					# allow creating static and shared libs without conflicts
        OUTPUT_NAME "rekkon_mmal_camera${PROJECT_DLLVERSION}" 	# avoid conflicts between library and binary target names
)

TARGET_LINK_LIBRARIES(RekkonMMALCamera  ${REQUIRED_LIBRARIES}  )

INSTALL(TARGETS RekkonMMALCamera
        RUNTIME DESTINATION bin COMPONENT main
        LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT main
        ARCHIVE DESTINATION lib COMPONENT main)

target_compile_definitions(RekkonMMALCamera PRIVATE REKKONMMALCAMERA_LIBRARY)

#Installation of all header files
INSTALL(FILES ${public_hdrs_base}
        DESTINATION include/rekkon_mmal_camera
        COMPONENT main)


    # ----------------------------------------------------------------------------
    # display status message for important variables
    # ----------------------------------------------------------------------------
    message( STATUS )
    message( STATUS "-------------------------------------------------------------------------------" )
    message( STATUS "General configuration for ${PROJECT_NAME} ${PROJECT_VERSION}")
    message( STATUS "-------------------------------------------------------------------------------" )
    message( STATUS )
    message("    Built as dynamic libs?:" ${BUILD_SHARED_LIBS})
    message("    Compiler:"                   "${CMAKE_COMPILER}"   "${CMAKE_CXX_COMPILER}")
    message( STATUS "C++ flags (Release):       ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    message( STATUS "C++ flags (Debug):         ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    message( STATUS "CMAKE_CXX_FLAGS:         ${CMAKE_CXX_FLAGS}")
    message( STATUS "CMAKE_BINARY_DIR:         ${CMAKE_BINARY_DIR}")

    MESSAGE( STATUS )
    MESSAGE( STATUS "TARGET_PROCESSOR = ${TARGET_PROCESSOR}" )
    MESSAGE( STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}" )
    MESSAGE( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
    MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
    MESSAGE( STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}" )

    MESSAGE( STATUS )
    MESSAGE( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
    MESSAGE( STATUS "REQUIRED_LIBRARIES=${REQUIRED_LIBRARIES}")
    MESSAGE( STATUS )
    MESSAGE( STATUS )
    MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
    MESSAGE( STATUS )

