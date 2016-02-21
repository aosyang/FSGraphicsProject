//=============================================================================
// IResource.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render resource interface class
//=============================================================================

enum ResourceState
{
	RS_Empty,
	RS_EnqueuedForLoading,
	RS_Loaded,
};

enum ResourceType
{
	RT_Mesh,
	RT_Texture,
};

class RBaseResource
{
public:
	RBaseResource(ResourceType type) : m_State(RS_Empty), m_Type(type) {}
	virtual ~RBaseResource() = 0 {}

	ResourceState GetResourceState() const { return m_State; }
	ResourceType GetResourceType() const { return m_Type; }

	// Multi-threading loading complete callback
	//virtual void OnLoaded() = 0;
protected:
	ResourceState		m_State;
	ResourceType		m_Type;
};
