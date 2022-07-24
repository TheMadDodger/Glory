#pragma once
#include <string>
#include <functional>
#include <map>
#include <ImageData.h>
#include <Texture.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class FileDialogImage : public ImageData
	{
	public:
		FileDialogImage(uint8_t* data, int w, int h, char fmt);
		virtual ~FileDialogImage();

		virtual const void* GetPixels() override;

	private:
		virtual void BuildTexture() override;

	private:
		uint8_t* m_pData;
		int m_Width;
		int m_Height;
		char m_Format;
	};

	class FileDialog
	{
	public:
		static GLORY_EDITOR_API void Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir, std::function<void(const std::string&)> callback);
		static GLORY_EDITOR_API void Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir, std::function<void(const std::string&)> callback);

	private:
		static void Initialize();
		static void Update();

	private:
		friend class MainEditor;
		static std::function<void(const std::string&)> m_Callback;
		static std::string m_CurrentBrowserKey;
		static std::map<std::string, FileDialogImage*> m_pImages;
	};
}
