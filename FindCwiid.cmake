IF (CWIID_INCLUDE_DIR)
  # Already in cache, be silent
  SET(CWIID_FIND_QUIETLY TRUE)
ENDIF (CWIID_INCLUDE_DIR)

FIND_PATH(CWIID_INCLUDE_DIR cwiid.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(CWIID_LIBRARY
  NAMES cwiid
  PATHS /usr/lib /usr/local/lib
)

IF (CWIID_INCLUDE_DIR AND CWIID_LIBRARY)
  SET(CWIID_FOUND TRUE)
  SET(CWIID_LIBRARIES ${CWIID_LIBRARY})
ENDIF (CWIID_INCLUDE_DIR AND CWIID_LIBRARY)

IF (CWIID_FOUND)
  IF (NOT CWIID_FIND_QUIETLY)
    MESSAGE(STATUS "Found cwiid: ${CWIID_LIBRARY}")
  ENDIF (NOT CWIID_FIND_QUIETLY)
ELSE (CWIID_FOUND)
  IF (CWIID_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for cwiid librarie.")
    MESSAGE(FATAL_ERROR "Could NOT find cwiid library")
  ENDIF (CWIID_FIND_REQUIRED)
ENDIF (CWIID_FOUND)

MARK_AS_ADVANCED(
  CWIID_LIBRARY
  CWIID_INCLUDE_DIR
  )
