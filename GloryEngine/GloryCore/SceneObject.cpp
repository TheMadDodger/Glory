#include "SceneObject.h"

namespace Glory
{
	SceneObject::SceneObject() : m_Name("Empty Object")
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::SceneObject(const std::string& name) : m_Name(name)
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::SceneObject(const std::string& name, UUID uuid) : m_Name(name), Object(uuid)
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::~SceneObject()
	{
	}

	const std::string& SceneObject::Name()
	{
		return m_Name;
	}

	void SceneObject::SetName(const std::string& name)
	{
		m_Name = name;
	}
}
