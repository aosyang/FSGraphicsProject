CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(RhinoEngine)

# Recursively find all .cpp and .h files
FILE(GLOB SRC
	 "*.cpp"
	 "*.h"
)

FILE(GLOB_RECURSE SRC_COLLISION
	 "Collision/*.cpp"
	 "Collision/*.h"
)

FILE(GLOB_RECURSE SRC_CORE
	 "Core/*.cpp"
	 "Core/*.h"
)

FILE(GLOB_RECURSE SRC_RENDERSYSTEM
	 "RenderSystem/*.cpp"
	 "RenderSystem/*.h"
)

FILE(GLOB_RECURSE SRC_RESOURCE
	 "Resource/*.cpp"
	 "Resource/*.h"
)

FILE(GLOB_RECURSE SRC_SCENE
	 "Scene/*.cpp"
	 "Scene/*.h"
)

FILE(GLOB_RECURSE SRC_SCRIPT
	 "Script/*.cpp"
	 "Script/*.h"
)

FILE(GLOB_RECURSE SRC_TINYXML2
	 "tinyxml2/*.cpp"
	 "tinyxml2/*.h"
)

SET(SRC_LUA_DIR ${CMAKE_SOURCE_DIR}/lua5.3)
FILE(GLOB_RECURSE SRC_LUA5_3
	 "${SRC_LUA_DIR}/*.c"
	 "${SRC_LUA_DIR}/*.h"
	 "${SRC_LUA_DIR}/*.hpp"
)

# Exclude lua source codes with a main function too avoid linking errors
LIST(REMOVE_ITEM SRC_LUA5_3 "${SRC_LUA_DIR}/lua.c")
LIST(REMOVE_ITEM SRC_LUA5_3 "${SRC_LUA_DIR}/luac.c")

INCLUDE_DIRECTORIES("." ${CMAKE_SOURCE_DIR}/fbx/include)

ADD_LIBRARY(RhinoEngine
	${SRC}
	${SRC_COLLISION}
	${SRC_CORE}
	${SRC_RENDERSYSTEM}
	${SRC_RESOURCE}
	${SRC_SCENE}
	${SRC_SCRIPT}
	${SRC_TINYXML2}
	${SRC_LUA5_3}
)

TARGET_LINK_LIBRARIES(RhinoEngine libfbxsdk.lib)

# Generate filters for the project file
SOURCE_GROUP("" FILES ${SRC})
SOURCE_GROUP("Collision" FILES ${SRC_COLLISION})
SOURCE_GROUP("Core" FILES ${SRC_CORE})
SOURCE_GROUP("RenderSystem" FILES ${SRC_RENDERSYSTEM})
SOURCE_GROUP("Resource" FILES ${SRC_RESOURCE})
SOURCE_GROUP("Scene" FILES ${SRC_SCENE})
SOURCE_GROUP("Script" FILES ${SRC_SCRIPT})
SOURCE_GROUP("tinyxml2" FILES ${SRC_TINYXML2})
SOURCE_GROUP("lua5.3" FILES ${SRC_LUA5_3})
