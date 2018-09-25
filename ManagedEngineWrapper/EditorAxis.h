//=============================================================================
// EditorAxis.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

enum EAxis
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,

	AXIS_COUNT,
};

class EditorAxis : public RSceneObject
{
	friend RScene;
public:
	virtual void Draw() override;
	const RAabb& GetAabb(int index) const;

protected:
	EditorAxis(const RConstructingParams& Params);
	virtual ~EditorAxis();

	void Create();
	void Release();

private:
	RMeshRenderBuffer			m_AxisMeshBuffer[AXIS_COUNT];
	RAabb						m_AxisAabb[AXIS_COUNT];
	ID3D11InputLayout*			m_ColorInputLayout;

	RShader*					m_ColorShader;
};

