#pragma once
#include "EditorShaderData.h"
#include <ShaderSourceData.h>
#include <ThreadManager.h>
#include <map>
#include <filesystem>
#include <shaderc.hpp>

namespace Glory::Editor
{
	class EditorShaderProcessor
	{
	public:
		static EditorShaderData* GetShaderSource(ShaderSourceData* pShaderSource);

	private:
		EditorShaderProcessor();
		virtual ~EditorShaderProcessor();

		void Start();
		void Stop();
		void ThreadLoop();


		void CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path);
		void LoadCache(ShaderSourceData* pShaderSource, std::filesystem::path path);

	private:
		friend class EditorApplication;
		static std::map<UUID, EditorShaderData*> m_pLoadedShaderData;
		static std::queue<ShaderSourceData*> m_ProcessShadersQueue;
		static std::condition_variable m_QueueCondition;
		static std::mutex m_QueueLock;
		bool m_Exit = false;
		Thread* m_pThread;

		std::map<ShaderType, shaderc_shader_kind> m_ShaderTypeToKind;
	};
}
