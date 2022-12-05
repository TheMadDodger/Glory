#include "pch.h"
#include "ProjectLock.h"

namespace Glory::Editor
{
	ProjectLock::ProjectLock(const std::string& path) : m_ProjectPath(path)
	{
		m_ProjectLockPath = path;
		m_ProjectLockPath = m_ProjectLockPath.parent_path().append("Project.lock");
	}

	ProjectLock::~ProjectLock()
	{
		Unlock();
	}
	
	bool ProjectLock::Lock()
	{
		if (m_FileStream.is_open()) return false;

		if (!std::filesystem::exists(m_ProjectLockPath))
		{
			m_FileStream.open(m_ProjectLockPath, std::ios::out);
			m_FileStream.close();
		}

		m_FileStream.open(m_ProjectLockPath, std::ios::out | std::ios::in);
		return m_FileStream.is_open();
	}

	void ProjectLock::Unlock()
	{
		if (!m_FileStream.is_open()) return;
		m_FileStream.close();
		std::filesystem::remove(m_ProjectLockPath);
	}

	bool ProjectLock::CanLock()
	{
		if (m_FileStream.is_open()) return false;

		if (!std::filesystem::exists(m_ProjectLockPath))
			return true;

		m_FileStream.open(m_ProjectLockPath, std::ios::out | std::ios::in);
		bool canLock = m_FileStream.is_open();
		m_FileStream.close();
		return canLock;
	}
}
