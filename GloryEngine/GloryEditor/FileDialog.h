#pragma once
#include <string>
#include <functional>

namespace Glory::Editor
{
	class FileDialog
	{
	public:
		static void Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir, std::function<void(const std::string&)> callback);
		static void Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir, std::function<void(const std::string&)> callback);

	private:
		static void Update();

	private:
		friend class MainEditor;
		static std::function<void(const std::string&)> m_Callback;
		static std::string m_CurrentBrowserKey;
	};
}
