//=============================================================================
// EditorCommon.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "EditorCommon.h"

RVec2 GetMousePositionInViewport()
{
	RECT ClientRect = GEngine.GetClientRectInfo();

	int CurX, CurY;
	RInput.GetCursorClientPosition(CurX, CurY);

	// Mouse cursor relative position in viewport
	return RVec2(
		float(CurX - ClientRect.left) / float(ClientRect.right - ClientRect.left),
		float(CurY - ClientRect.top) / float(ClientRect.bottom - ClientRect.top));
}

RRay MakeRayFromViewportPoint(RCamera* Camera, const RVec2& Point)
{
	RVec4 FarPoint = RVec4(2.0f * Point.x - 1.0f, -2.0f * Point.y + 1.0f, 1.0f, 1.0f);
	FarPoint = FarPoint * Camera->GetViewProjMatrix().Inverse();
	RVec3 FarPointWorld = FarPoint.ToVec3() / FarPoint.w;
	RVec3 CameraPosition = Camera->GetPosition();

	return RRay(CameraPosition, FarPointWorld);
}
