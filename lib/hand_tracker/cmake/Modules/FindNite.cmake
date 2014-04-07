# - Try to find Nite
# Once done, this will define
#
#  Nite_FOUND - system has Nite
#  Nite_INCLUDE_DIRS - the Nite include directories
#  Nite_LIBRARIES - link these to use Nite

include(LibFindMacros)

IF (UNIX)

  IF(NOT NITE_ROOT)
    IF(EXISTS "/usr/include/nite")
  	  SET(NITE_ROOT "/usr")
    ELSEIF(EXISTS "/usr/local/include/nite")
  	  SET(NITE_ROOT "/usr/local")
    ELSE()
      MESSAGE("NITE_ROOT not set. Continuing anyway..")
    ENDIF()
  ENDIF()

	# Use pkg-config to get hints about paths
	libfind_pkg_check_modules(Nite_PKGCONF libNite)
	# Include dir
	find_path(Nite_INCLUDE_DIR
	  NAMES XnVNite.h
	  PATHS ${Nite_PKGCONF_INCLUDE_DIRS} ${NITE_ROOT}/include/nite
	)
	# Finally the library itself (TODO: check versions)
	find_library(Nite_LIBRARY
	  NAMES XnVNite_1_3_1
	  PATHS ${Nite_PKGCONF_LIBRARY_DIRS} ${NITE_ROOT}/lib
	)

ELSEIF (WIN32)
	find_path(Nite_INCLUDE_DIR
	  NAMES XnNite.h
	  PATHS "${NITE_ROOT}/Include" "C:/Program Files (x86)/Nite/Include" "C:/Program Files/Nite/Include" ${CMAKE_INCLUDE_PATH}
	)
	# Finally the library itself
	find_library(Nite_LIBRARY
	  NAMES Nite
	  PATHS "${NITE_ROOT}/Lib" "C:/Program Files (x86)/Nite/Lib" "C:/Program Files/Nite/Lib" ${CMAKE_LIB_PATH}
	)
ENDIF()
	
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Nite_PROCESS_INCLUDES Nite_INCLUDE_DIR Nite_INCLUDE_DIRS)
set(Nite_PROCESS_LIBS Nite_LIBRARY Nite_LIBRARIES)
libfind_process(Nite)
