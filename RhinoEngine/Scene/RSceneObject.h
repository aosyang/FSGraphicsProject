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

	const RMatrix4& GetNodeTransform() const;

	void SetPosition(const RVec3& pos);

	virtual void Draw();

private:
	RMatrix4	m_NodeTransform;
};

#endif
