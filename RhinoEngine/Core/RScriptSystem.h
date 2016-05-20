//=============================================================================
// RScriptSystem.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RSCRIPTSYSTEM_H
#define _RSCRIPTSYSTEM_H

class RSceneObject;

class RScriptSystem : public RSingleton<RScriptSystem>
{
	friend class RSingleton<RScriptSystem>;
	RScriptSystem();
public:
	bool Initialize();
	void Shutdown();

	void RegisterFunction(const char* func_name, lua_CFunction func);
	bool Start();

	void RegisterScriptableObject(RSceneObject* obj);
	void UnregisterScriptableObject(RSceneObject* obj);
	void UpdateScriptableObjects();
private:
	lua_State*					m_LuaState;
	vector<RSceneObject*>		m_ScriptableObjects;
};

#define RScript RScriptSystem::Instance()

#endif
