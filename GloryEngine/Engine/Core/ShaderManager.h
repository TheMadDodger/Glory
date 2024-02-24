#pragma once
#include "UUID.h"
#include "GraphicsEnums.h"

namespace Glory
{
	class Engine;
	class FileData;
	class ShaderSourceData;

	class ShaderManager
	{
	public:
		ShaderManager(Engine* pEngine);
		virtual ~ShaderManager() = default;

	public:
		virtual ShaderType GetShaderType(UUID shaderID) const = 0;
		virtual FileData* GetCompiledShaderFile(UUID shaderID) const = 0;

		virtual void AddShader(ShaderSourceData*) {};

	protected:
		friend class Engine;
		Engine* m_pEngine;
	};
}
