#include "FileDialog.h"
#include <ImFileDialog.h>

namespace Glory::Editor
{
	std::function<void(const std::string&)> FileDialog::m_Callback;
	std::string FileDialog::m_CurrentBrowserKey;

	void FileDialog::Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir, std::function<void(const std::string&)> callback)
	{
		m_CurrentBrowserKey = key;
		m_Callback = callback;
		ifd::FileDialog::Instance().Save(key, title, filter, startingDir);
	}

	void FileDialog::Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir, std::function<void(const std::string&)> callback)
	{
		m_CurrentBrowserKey = key;
		m_Callback = callback;
		ifd::FileDialog::Instance().Open(key, title, filter, isMultiselect, startingDir);
	}

	void FileDialog::Update()
	{
		if (ifd::FileDialog::Instance().IsDone(m_CurrentBrowserKey))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				std::string result = ifd::FileDialog::Instance().GetResult().u8string();
				m_Callback(result);
			}
			ifd::FileDialog::Instance().Close();
		}
	}
}
