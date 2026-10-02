CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

# Name
PROJECT( SCED )

# Version
SET( ${PROJECT_NAME}_VERSION_MAJOR 0 )
SET( ${PROJECT_NAME}_VERSION_MINOR 0 )
SET( ${PROJECT_NAME}_VERSION_PATCH 1 )

# Dependencies
FIND_PACKAGE( SDL3 REQUIRED CONFIG )
FIND_PACKAGE( SDL3_ttf REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( glm REQUIRED )
FIND_PACKAGE( Fontconfig REQUIRED )

# Default to an optimized build with debug information
IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()

# Enforce the language standard
SET( CMAKE_CXX_STANDARD 17 )
SET( CMAKE_CXX_STANDARD_REQUIRED ON )

# Include directories
INCLUDE_DIRECTORIES( BEFORE ./include )
INCLUDE_DIRECTORIES( BEFORE ./src )
INCLUDE_DIRECTORIES( BEFORE ./src/server )
INSTALL( DIRECTORY ./include/ DESTINATION include FILES_MATCHING PATTERN "*.h" )

# Compile options
ADD_COMPILE_OPTIONS( "-Wall" "-Wextra" )

# SCED CLIENT 

# Collect client source files
AUX_SOURCE_DIRECTORY( ./src/client client_sources )

# Compile client
ADD_LIBRARY( sced-client SHARED ${client_sources} )

INSTALL( TARGETS sced-client LIBRARY DESTINATION lib )

# SCED SERVER 

# Collect server source files
AUX_SOURCE_DIRECTORY( ./src/server server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/utils server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/third_party server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/ui server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/draw server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/draw/elements server_sources )
AUX_SOURCE_DIRECTORY( ./src/server/draw/elements/helpers server_sources )

# Compile server
ADD_EXECUTABLE( sced-server ${server_sources} )

# Link libraries
TARGET_LINK_LIBRARIES(
  sced-server
  sced-client
  SDL3::SDL3
  OpenGL::GL
  SDL3_ttf::SDL3_ttf
  glm::glm
  Fontconfig::Fontconfig
)

INSTALL( TARGETS sced-server DESTINATION bin )