CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(RhinoLevelEditor VERSION 0.1.0 LANGUAGES CSharp)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cs"
	 "*.h"
)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_EXECUTABLE(RhinoLevelEditor WIN32 ${SRC})
ADD_DEPENDENCIES(RhinoLevelEditor RhinoEngine ManagedInterface ManagedEngineWrapper)
TARGET_LINK_LIBRARIES(RhinoLevelEditor RhinoEngine libfbxsdk.lib)
SET_TARGET_PROPERTIES(RhinoLevelEditor PROPERTIES COMMON_LANGUAGE_RUNTIME "")
SET_TARGET_PROPERTIES(RhinoLevelEditor PROPERTIES VS_DOTNET_REFERENCES "System;System.Core;System.Data;System.Drawing;System.Windows.Forms;System.Xml")
SET_PROPERTY(TARGET RhinoLevelEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/RhinoLevelEditor")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET RhinoLevelEditor
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:RhinoLevelEditor>/.
)
