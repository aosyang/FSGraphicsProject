//=============================================================================
// RScriptSystem.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RScriptSystem.h"

void print_error(lua_State* state) {
	// The error message is on top of the stack.
	// Fetch it, print it and then pop it off the stack.
	const char* message = lua_tostring(state, -1);
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
	lua_pop(state, 1);
}

RScriptSystem::RScriptSystem()
	: m_LuaState(nullptr)
{

}

bool RScriptSystem::Initialize()
{
	m_LuaState = luaL_newstate();
	
	// Use standard library in lua scripts
	luaL_openlibs(m_LuaState);

	int result = luaL_loadfile(m_LuaState, "../Scripts/GameMain.lua");

	if (result != LUA_OK) {
		print_error(m_LuaState);
		return false;
	}

	RSceneObject::RegisterScriptFunctions();

	return true;
}

void RScriptSystem::Shutdown()
{
	lua_close(m_LuaState);
}

bool RScriptSystem::Start()
{
	// Run script once for function calling from c++
	int result = lua_pcall(m_LuaState, 0, LUA_MULTRET, 0);

	if (result != LUA_OK) {
		print_error(m_LuaState);
		return false;
	}

	return true;
}

void RScriptSystem::RegisterFunction(const char* func_name, lua_CFunction func)
{
	lua_register(m_LuaState, func_name, func);
}

void RScriptSystem::RegisterScriptableObject(RSceneObject* obj)
{
	if (find(m_ScriptableObjects.begin(), m_ScriptableObjects.end(), obj) == m_ScriptableObjects.end())
	{
		m_ScriptableObjects.push_back(obj);
	}
}

void RScriptSystem::UnregisterScriptableObject(RSceneObject* obj)
{
	vector<RSceneObject*>::iterator iter = find(m_ScriptableObjects.begin(), m_ScriptableObjects.end(), obj);
	if (iter != m_ScriptableObjects.end())
		m_ScriptableObjects.erase(iter);
}

void RScriptSystem::UpdateScriptableObjects()
{
	for (vector<RSceneObject*>::iterator iter = m_ScriptableObjects.begin(); iter != m_ScriptableObjects.end(); iter++)
	{
		const char* script = (*iter)->GetScript();
		if (!script || script[0] == 0)
			continue;

		lua_getglobal(m_LuaState, script);
		if (lua_isfunction(m_LuaState, -1))
		{
			lua_pushlightuserdata(m_LuaState, *iter);
			lua_pcall(m_LuaState, 1, 0, 0);
		}
	}
}
