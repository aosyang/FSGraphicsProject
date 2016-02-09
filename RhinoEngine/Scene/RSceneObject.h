//=============================================================================
// RSceneObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSCENEOBJECT_H
#define _RSCENEOBJECT_H

class RSceneObject
{
public:
	RSceneObject();
	virtual ~RSceneObject();

	XMMATRIX GetNodeTransform() const;

	void SetPosition(const XMFLOAT3& pos);

	virtual void Draw();

private:
	XMFLOAT4X4	m_NodeTransform;
};

#endif
