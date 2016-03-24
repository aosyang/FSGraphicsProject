//=============================================================================
// EditorAxis.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _EDITORAXIS_H
#define _EDITORAXIS_H

enum EAxis
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,

	AXIS_COUNT,
};

class EditorAxis
{
private:
	RMeshElement				m_AxisMeshBuffer[AXIS_COUNT];
	RAabb						m_AxisAabb[AXIS_COUNT];

public:
	EditorAxis();

	void Create();
	void Release();

	void Draw();
	const RAabb& GetAabb(int index) const;
};

#endif
