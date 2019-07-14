CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(SimpleGameProject)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_EXECUTABLE(SimpleGameProject WIN32 ${SRC})
ADD_DEPENDENCIES(SimpleGameProject RhinoEngine)
TARGET_LINK_LIBRARIES(SimpleGameProject RhinoEngine)
TARGET_LINK_LIBRARIES(SimpleGameProject libfbxsdk.lib)
SET_PROPERTY(TARGET SimpleGameProject PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/SimpleGameProject")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET SimpleGameProject
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:SimpleGameProject>/.
)
