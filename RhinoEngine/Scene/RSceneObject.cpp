//=============================================================================
// RSceneObject.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSceneObject.h"

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


void RSceneObject::RegisterScriptFunctions()
{
	RScript.RegisterFunction("MoveTo",	ScriptFunc_MoveTo,	{ { SPT_Float, SPT_Float, SPT_Float } });
	RScript.RegisterFunction("Swing",	ScriptFunc_Swing,	{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
	RScript.RegisterFunction("Rotate",	ScriptFunc_Rotate,	{ { SPT_Float, SPT_Float, SPT_Float, SPT_Float } });
}

RSceneObject::RSceneObject()
	: m_Scene(nullptr)
{
	m_NodeTransform = RMatrix4::IDENTITY;
	RScript.RegisterScriptableObject(this);
}

RSceneObject::~RSceneObject()
{
	RScript.UnregisterScriptableObject(this);
}

const RMatrix4& RSceneObject::GetNodeTransform() const
{
	return m_NodeTransform;
}

void RSceneObject::SetTransform(const RMatrix4& transform)
{
	m_NodeTransform = transform;
}

void RSceneObject::SetRotation(const RMatrix4& rot)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m_NodeTransform.m[i][j] = rot.m[i][j];
}

void RSceneObject::SetPosition(const RVec3& pos)
{
	m_NodeTransform.SetTranslation(pos);
}

RVec3 RSceneObject::GetPosition() const
{
	return m_NodeTransform.GetRow(3).ToVec3();
}

void RSceneObject::LookAt(const RVec3 target)
{
	RVec3 pos = m_NodeTransform.GetTranslation();
	RVec3 forward = target - pos;
	forward.Normalize();
	RVec3 right = RVec3(0, 1, 0).Cross(forward);
	right.Normalize();
	RVec3 up = forward.Cross(right);

	m_NodeTransform.SetRow(0, RVec4(right, 0));
	m_NodeTransform.SetRow(1, RVec4(up, 0));
	m_NodeTransform.SetRow(2, RVec4(forward, 0));
	m_NodeTransform.SetRow(3, RVec4(pos, 1));
}

void RSceneObject::Translate(const RVec3& v)
{
	m_NodeTransform.Translate(v);
}

void RSceneObject::TranslateLocal(const RVec3& v)
{
	m_NodeTransform.TranslateLocal(v);
}

const vector<string>& RSceneObject::GetParsedScript()
{
	if (!m_Script.empty() && m_ParsedScript.empty())
	{
		size_t pos = 0;
		string script = m_Script;

		while ((pos = script.find(',')) != string::npos)
		{
			script = script.replace(pos, 1, " ");
		}

		istringstream iss(script);
		copy(istream_iterator<string>(iss),
			istream_iterator<string>(),
			back_inserter(m_ParsedScript));
	}

	return m_ParsedScript;
}
