//=============================================================================
// RScriptSystem.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "RSingleton.h"

#include "../lua5.3/lua.hpp"

class RSceneObject;

enum ScriptParamType
{
	SPT_None,
	SPT_Float,
};

struct ScriptParams
{
	ScriptParamType type[10];
};

class RScriptSystem : public RSingleton<RScriptSystem>
{
	friend class RSingleton<RScriptSystem>;
	RScriptSystem();
public:
	bool Initialize();
	void Shutdown();

	void RegisterFunction(const char* func_name, lua_CFunction func, ScriptParams paramTypes);
	bool Start();

	void RegisterScriptableObject(RSceneObject* obj);
	void UnregisterScriptableObject(RSceneObject* obj);
	void UpdateScriptableObjects();
private:
	lua_State*					m_LuaState;
	std::vector<RSceneObject*>		m_ScriptableObjects;
	std::map<std::string, ScriptParams>	m_ScriptParams;
};

#define GScriptSystem RScriptSystem::Instance()

