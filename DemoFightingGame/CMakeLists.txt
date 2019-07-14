CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(DemoFightingGame)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_EXECUTABLE(DemoFightingGame WIN32 ${SRC})
ADD_DEPENDENCIES(DemoFightingGame RhinoEngine)
TARGET_LINK_LIBRARIES(DemoFightingGame RhinoEngine)
TARGET_LINK_LIBRARIES(DemoFightingGame libfbxsdk.lib)
SET_PROPERTY(TARGET DemoFightingGame PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/DemoFightingGame")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET DemoFightingGame
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:DemoFightingGame>/.
)
