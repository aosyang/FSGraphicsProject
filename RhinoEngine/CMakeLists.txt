CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(RhinoEngine)

# Shader compilation options
FOREACH(Shader ${SRC_HLSL})
	SET_SOURCE_FILES_PROPERTIES(${Shader} PROPERTIES
		VS_SHADER_TYPE Vertex
		VS_SHADER_MODEL 4.0
		VS_SHADER_ENTRYPOINT main
		VS_SHADER_OBJECT_FILE_NAME " "
		VS_SHADER_VARIABLE_NAME "%(Filename)"
		VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_SOURCE_DIR}/RhinoEngine/%(Filename).csh"
		VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
		VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
	)
ENDFOREACH()

# Adding shader files
FILE(GLOB_RECURSE SRC_SHADERS
	 "../Shaders/*.h"
	 "../Shaders/*.hlsl"
	 "../Shaders/*.hlsli"
)

# Exclude shaders from the building process as they're only for editing
FOREACH(Shader ${SRC_SHADERS})
	SET_SOURCE_FILES_PROPERTIES(${Shader} PROPERTIES
		HEADER_FILE_ONLY TRUE
	)
ENDFOREACH()

# Adding source files of lua 5.3
SET(SRC_LUA_DIR ${CMAKE_SOURCE_DIR}/lua5.3)
FILE(GLOB_RECURSE SRC_LUA5_3
	 "${SRC_LUA_DIR}/*.c"
	 "${SRC_LUA_DIR}/*.h"
	 "${SRC_LUA_DIR}/*.hpp"
)

# Exclude lua source codes with a main function too avoid linking errors
LIST(REMOVE_ITEM SRC_LUA5_3 "${SRC_LUA_DIR}/lua.c")
LIST(REMOVE_ITEM SRC_LUA5_3 "${SRC_LUA_DIR}/luac.c")

INCLUDE_DIRECTORIES("." ${CMAKE_SOURCE_DIR}/ThirdParty/fbxsdk/include ${CMAKE_SOURCE_DIR}/ThirdParty/Bullet3/src)

# Collection of engine source code files
SET(SRC_ROOT_PATH "${CMAKE_SOURCE_DIR}/RhinoEngine")
FILE(
    GLOB_RECURSE SOURCE_LIST 
    LIST_DIRECTORIES FALSE
    "${SRC_ROOT_PATH}/*.c"
    "${SRC_ROOT_PATH}/*.cpp"
    "${SRC_ROOT_PATH}/*.h"
    "${SRC_ROOT_PATH}/*.hpp"
	"${SRC_ROOT_PATH}/*.hlsl"
)

ADD_LIBRARY(${PROJECT_NAME} STATIC
	${SOURCE_LIST}
	${SRC_LUA5_3}
	${SRC_SHADERS}
)

# Put source files into filters base on their paths
FOREACH(SOURCE IN ITEMS ${SOURCE_LIST})
    GET_FILENAME_COMPONENT(SRC_PATH "${SOURCE}" PATH)
    FILE(RELATIVE_PATH SRC_REL_PATH "${SRC_ROOT_PATH}" "${SRC_PATH}")
    STRING(REPLACE "/" "\\" _GROUP_PATH "${SRC_REL_PATH}")
    SOURCE_GROUP("${_GROUP_PATH}" FILES "${SOURCE}")
ENDFOREACH()

# Linking fbx sdk
TARGET_LINK_LIBRARIES(${PROJECT_NAME} libfbxsdk.lib)

# Generate filters for external source files and shaders
SOURCE_GROUP("lua5.3" FILES ${SRC_LUA5_3})
SOURCE_GROUP("Shaders" FILES ${SRC_SHADERS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES FOLDER Engine)
