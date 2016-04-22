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

	void SetTransform(const RMatrix4& transform);
	void SetPosition(const RVec3& pos);
	RVec3 GetPosition() const;

	void Translate(const RVec3& v);
	void TranslateLocal(const RVec3& v);

	virtual void Draw();

protected:
	RMatrix4	m_NodeTransform;
};

#endif
