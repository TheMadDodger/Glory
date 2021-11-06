#include "SceneObject.h"

namespace Glory
{
	SceneObject::SceneObject() : m_Name("Empty Object")
	{
	}

	SceneObject::SceneObject(const std::string& name) : m_Name(name)
	{
	}

	SceneObject::SceneObject(const std::string& name, UUID uuid) : m_Name(name), Object(uuid)
	{
	}

	SceneObject::~SceneObject()
	{
	}

	const std::string& SceneObject::Name()
	{
		return m_Name;
	}
}
