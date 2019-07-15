CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(ManagedEngineWrapper)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

# Use precompiled headers
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Yu")

SET_SOURCE_FILES_PROPERTIES(Stdafx.cpp PROPERTIES COMPILE_FLAGS /Yc)

INCLUDE_DIRECTORIES(${RHINO_ENGINE_INCLUDE_DIR})
ADD_LIBRARY(ManagedEngineWrapper SHARED ${SRC})
ADD_DEPENDENCIES(ManagedEngineWrapper RhinoEngine ManagedInterface)
TARGET_LINK_LIBRARIES(ManagedEngineWrapper RhinoEngine libfbxsdk.lib)
SET_TARGET_PROPERTIES(ManagedEngineWrapper PROPERTIES COMMON_LANGUAGE_RUNTIME "")
SET_TARGET_PROPERTIES(ManagedEngineWrapper PROPERTIES VS_DOTNET_REFERENCES "System;System.Core;System.Data;System.Drawing;System.Xml")
SET_PROPERTY(TARGET ManagedEngineWrapper PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/ManagedEngineWrapper")

# Copy libfbxsdk.dll to the executable directory
ADD_CUSTOM_COMMAND(TARGET ManagedEngineWrapper
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FBX_LIB_DIR}/$<CONFIG>/libfbxsdk.dll
		$<TARGET_FILE_DIR:ManagedEngineWrapper>/.
)
