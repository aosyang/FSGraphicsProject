CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(DemoDeferredShading)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_EXECUTABLE(DemoDeferredShading WIN32 ${SRC})
ADD_DEPENDENCIES(DemoDeferredShading RhinoEngine)
TARGET_LINK_LIBRARIES(DemoDeferredShading RhinoEngine)
TARGET_LINK_LIBRARIES(DemoDeferredShading libfbxsdk.lib)
SET_PROPERTY(TARGET DemoDeferredShading PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/DemoDeferredShading")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET DemoDeferredShading
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:DemoDeferredShading>/.
)
