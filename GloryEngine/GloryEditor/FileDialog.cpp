#include "FileDialog.h"
#include "EditorAssets.h"
#include "EditorApplication.h"
#include <ImFileDialog.h>
#include <Game.h>

namespace Glory::Editor
{
	std::function<void(const std::string&)> FileDialog::m_Callback;
	std::string FileDialog::m_CurrentBrowserKey;
	std::map<std::string, FileDialogImage*> FileDialog::m_pImages;

	GLORY_EDITOR_API void FileDialog::Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir, std::function<void(const std::string&)> callback)
	{
		m_CurrentBrowserKey = key;
		m_Callback = callback;
		ifd::FileDialog::Instance().Save(key, title, filter, startingDir);
	}

	GLORY_EDITOR_API void FileDialog::Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir, std::function<void(const std::string&)> callback)
	{
		m_CurrentBrowserKey = key;
		m_Callback = callback;
		ifd::FileDialog::Instance().Open(key, title, filter, isMultiselect, startingDir);
	}

	void FileDialog::Initialize()
	{
		ifd::FileDialog::Instance().CreateTexture = [&](const std::string& path, uint8_t* data, int w, int h, char fmt) -> void*
		{
			bool exists = true;
			
			if (m_pImages.find(path) == m_pImages.end())
			{
				m_pImages[path] = new FileDialogImage(data, w, h, fmt);
				exists = false;
			}
		
			GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();
			EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		
			FileDialogImage* pImage = m_pImages[path];
			if (exists)
			{
				if (pResourceManager->ResourceExists(pImage))
				{
					Texture* pTexture = pResourceManager->CreateTexture(pImage);
					return pRenderImpl->GetTextureID(pTexture);
				}
				return 0;
			}
			EditorAssets::EnqueueTextureCreation(pImage);
			return 0;
		
			//GLuint tex;
			//glGenTextures(1, &tex);
			//LogGLError(glGetError(), true);
			//glBindTexture(GL_TEXTURE_2D, tex);
			//LogGLError(glGetError(), true);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//LogGLError(glGetError(), true);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//LogGLError(glGetError(), true);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//LogGLError(glGetError(), true);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//LogGLError(glGetError(), true);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
			//LogGLError(glGetError(), true);
			//glGenerateMipmap(GL_TEXTURE_2D);
			//LogGLError(glGetError(), true);
			//glBindTexture(GL_TEXTURE_2D, 0);
			//LogGLError(glGetError(), true);
		
			//return 0;//(void*)tex;
		};
		ifd::FileDialog::Instance().DeleteTexture = [&](void* tex) {
			//GLuint texID = (GLuint)tex;
			//glDeleteTextures(1, &texID);
			//LogGLError(glGetError(), true);
		};
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

	FileDialogImage::FileDialogImage(uint8_t* data, int w, int h, char fmt) : m_pData(data), m_Width(w), m_Height(h), m_Format(fmt)
	{
	}

	FileDialogImage::~FileDialogImage()
	{
	}

	const void* FileDialogImage::GetPixels()
	{
		return (const void*)m_pData;
	}

	void FileDialogImage::BuildTexture()
	{
		m_Width = static_cast<uint32_t>(m_Width);
		m_Height = static_cast<uint32_t>(m_Height);

		m_PixelFormat = Glory::PixelFormat::PF_R8G8B8Srgb;
		m_BytesPerPixel = 1;
	}
}
