cmake_minimum_required( VERSION 2.6 )

SET (CMAKE_VERBOSE_MAKEFILE OFF)

# project wide build settings
# compiler flags
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -Wall")

PROJECT (TRAIN)

SET (EXEC TRAIN)	#executable name
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#OpenCV
FIND_PACKAGE( OpenCV REQUIRED )
IF(OPENCV_FOUND)
	MESSAGE("OPENCV information:") 
	MESSAGE("  OPENCV_INCLUDE_DIRS: ${OPENCV_INCLUDE_DIRS} ")
	MESSAGE("  OPENCV_LIBRARIES: ${OPENCV_LIBRARIES}")
	MESSAGE("=========================================\n")
ELSE()
    MESSAGE("OPENCV NOT FOUND!")
	MESSAGE("=========================================\n")
ENDIF()
	
#OpenNI	
FIND_PACKAGE( OpenNI REQUIRED )
IF(OPENNI_FOUND)
	MESSAGE("OPENNI information:") 
	MESSAGE("  OPENNI_INCLUDE_DIRS: ${OPENNI_INCLUDE_DIRS} ")
	MESSAGE("  OPENNI_LIBRARIES: ${OPENNI_LIBRARIES}")
	MESSAGE("=========================================\n")
ELSE()
    MESSAGE("OPENNI NOT FOUND!")
	MESSAGE("=========================================\n")
ENDIF()
set(WITH_OPENNI ON CACHE BOOL "Include OpenNI support")

LINK_DIRECTORIES(${OPENNI_LIBRARIES})

#Nite
FIND_PACKAGE( Nite REQUIRED )
IF(NITE_FOUND)
	MESSAGE("Nite information:") 
	MESSAGE("  Nite_INCLUDE_DIRS: ${Nite_INCLUDE_DIRS} ")
	MESSAGE("  Nite_LIBRARIES: ${Nite_LIBRARIES}")
	MESSAGE("=========================================\n")
ELSE()
    MESSAGE("NITE NOT FOUND!")
	MESSAGE("=========================================\n")
ENDIF()
	
#OpenMP
FIND_PACKAGE(OpenMP REQUIRED)
IF(OPENMP_FOUND)
    MESSAGE("OPENMP FOUND")
	MESSAGE("=========================================\n")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ELSE()
    MESSAGE("OPENMP NOT FOUND!")
	MESSAGE("=========================================\n")
ENDIF()

#GTK2
FIND_PACKAGE(GTK2 REQUIRED)
IF(GTK2_FOUND)
    MESSAGE("GTK2 FOUND")
    MESSAGE("    GTK2_INCLUDE_DIRS: ${GTK2_INCLUDE_DIRS} ")
    MESSAGE("    GTK2_LIBRARIES: ${GTK2_LIBRARIES} ")
	MESSAGE("=========================================\n")
ELSE()
    MESSAGE("GTK2 NOT FOUND!")
	MESSAGE("=========================================\n")
ENDIF()

# include files from this project
INCLUDE_DIRECTORIES (${OpenCV_INCLUDE_DIRS}
                    ${OPENNI_INCLUDE_DIRS}
                    ${Nite_INCLUDE_DIRS}
                    ${GTK2_INCLUDE_DIRS})

SET (LIBS ${GLIB2_LIBRARIES}
		${OpenCV_LIBS}
		${OPENNI_LIBRARIES}
		${Nite_LIBRARIES}
		${GTK2_LIBRARIES})

# files in this dir + all above
SET (SRCS main.cpp
            tree.cpp
            grab.cpp
            kalmanfilter.cpp)

# the target
ADD_EXECUTABLE (${EXEC} ${SRCS})
# link all libs
TARGET_LINK_LIBRARIES (${EXEC} ${LIBS})
