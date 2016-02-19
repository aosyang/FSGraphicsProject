//=============================================================================
// IResource.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render resource interface class
//=============================================================================

class IResource
{
public:
	virtual ~IResource() = 0 {}

	// Multi-threading loading complete callback
	//virtual void OnLoaded() = 0;
};
