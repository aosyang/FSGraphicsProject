//=============================================================================
// RScriptedBehavior.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "RScriptedBehavior.h"


static int ScriptFunc_MoveTo(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 4)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 target(
		(float)lua_tonumber(state, 2),
		(float)lua_tonumber(state, 3),
		(float)lua_tonumber(state, 4));

	float sqrDist = (target - obj->GetPosition()).SquaredMagitude();
	if (sqrDist <= 1.0f)
		obj->SetPosition(target);
	else
	{
		RVec3 rel = (target - obj->GetPosition()).GetNormalized();
		if (rel.SquaredMagitude() > 0.0f)
			obj->Translate(rel);
	}

	return 0;
}

static int ScriptFunc_Swing(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 7)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 origin(
		(float)lua_tonumber(state, 2),
		(float)lua_tonumber(state, 3),
		(float)lua_tonumber(state, 4));

	RVec3 target(
		(float)lua_tonumber(state, 5),
		(float)lua_tonumber(state, 6),
		(float)lua_tonumber(state, 7));

	float amplitude = (float)lua_tonumber(state, 8);
	RVec3 pos = RVec3::Lerp(origin, target, sinf(GEngine.GetTimer().TotalTime()) * 0.5f + 0.5f);

	obj->SetPosition(pos);

	return 0;
}

static int ScriptFunc_Rotate(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 4)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 axis(
		(float)lua_tonumber(state, 2),
		(float)lua_tonumber(state, 3),
		(float)lua_tonumber(state, 4));

	float scale = 1.0f;
	if (n >= 5)
		scale = (float)lua_tonumber(state, 5);
	axis *= GEngine.GetTimer().TotalTime() * scale;

	RMatrix4 transform = obj->GetTransformMatrix();
	obj->SetRotation(RQuat::Euler(DEG_TO_RAD(axis.X()), DEG_TO_RAD(axis.Y()), DEG_TO_RAD(axis.Z())));

	return 0;
}

namespace ScriptedBehavior
{
	void RegisterScriptFunctions()
	{
		GScriptSystem.RegisterFunction("MoveTo", ScriptFunc_MoveTo,	{ { SPT_Float, SPT_Float, SPT_Float } });
		GScriptSystem.RegisterFunction("Swing", ScriptFunc_Swing,		{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
		GScriptSystem.RegisterFunction("Rotate", ScriptFunc_Rotate,	{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
	}
}
