CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(FSGraphicsProject)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_EXECUTABLE(FSGraphicsProject WIN32 ${SRC})
ADD_DEPENDENCIES(FSGraphicsProject RhinoEngine)
TARGET_LINK_LIBRARIES(FSGraphicsProject RhinoEngine)
TARGET_LINK_LIBRARIES(FSGraphicsProject libfbxsdk.lib)
SET_PROPERTY(TARGET FSGraphicsProject PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/FSGraphicsProject")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET FSGraphicsProject
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:FSGraphicsProject>/.
)
