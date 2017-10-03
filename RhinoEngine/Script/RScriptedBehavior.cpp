#include "Rhino.h"

#include "RScriptedBehavior.h"


static int ScriptFunc_MoveTo(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 4)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 target;
	target.x = (float)lua_tonumber(state, 2);
	target.y = (float)lua_tonumber(state, 3);
	target.z = (float)lua_tonumber(state, 4);

	float sqrDist = (target - obj->GetPosition()).SquaredMagitude();
	if (sqrDist <= 1.0f)
		obj->SetPosition(target);
	else
	{
		RVec3 rel = (target - obj->GetPosition()).GetNormalizedVec3();
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
	RVec3 origin, target;
	origin.x = (float)lua_tonumber(state, 2);
	origin.y = (float)lua_tonumber(state, 3);
	origin.z = (float)lua_tonumber(state, 4);
	target.x = (float)lua_tonumber(state, 5);
	target.y = (float)lua_tonumber(state, 6);
	target.z = (float)lua_tonumber(state, 7);

	float amplitude = (float)lua_tonumber(state, 8);
	RVec3 pos = RVec3::Lerp(origin, target, sinf(REngine::GetTimer().TotalTime()) * 0.5f + 0.5f);

	obj->SetPosition(pos);

	return 0;
}

static int ScriptFunc_Rotate(lua_State* state)
{
	int n = lua_gettop(state);

	if (n < 4)
		return 0;

	RSceneObject* obj = static_cast<RSceneObject*>(lua_touserdata(state, 1));
	RVec3 axis;
	axis.x = (float)lua_tonumber(state, 2);
	axis.y = (float)lua_tonumber(state, 3);
	axis.z = (float)lua_tonumber(state, 4);

	float scale = 1.0f;
	if (n >= 5)
		scale = (float)lua_tonumber(state, 5);
	axis *= REngine().GetTimer().TotalTime() * scale;

	RMatrix4 transform = obj->GetNodeTransform();
	obj->SetRotation(RMatrix4::CreateZAxisRotation(axis.z) *
					 RMatrix4::CreateYAxisRotation(axis.y) *
					 RMatrix4::CreateXAxisRotation(axis.x));

	return 0;
}

namespace ScriptedBehavior
{
	void RegisterScriptFunctions()
	{
		RScript.RegisterFunction("MoveTo", ScriptFunc_MoveTo,	{ { SPT_Float, SPT_Float, SPT_Float } });
		RScript.RegisterFunction("Swing", ScriptFunc_Swing,		{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
		RScript.RegisterFunction("Rotate", ScriptFunc_Rotate,	{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
	}
}
