#include "OGLPipeline.h"
#include "OpenGLGraphicsModule.h"
#include "GLShader.h"

#include <Engine.h>

namespace Glory
{
	OGLPipeline::OGLPipeline(PipelineData* pPipelineData): Pipeline(pPipelineData), m_ProgramID(NULL)
	{
	}

	OGLPipeline::~OGLPipeline()
	{
	}

	void OGLPipeline::Use()
	{
		glUseProgram(m_ProgramID);
	}

	GLuint OGLPipeline::ProgramID()
	{
		return m_ProgramID;
	}

	void OGLPipeline::Initialize()
	{
		int success;
		char infoLog[512];

		m_ProgramID = glCreateProgram();
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (size_t i = 0; i < m_pShaders.size(); ++i)
		{
			GLShader* pShader = (GLShader*)m_pShaders[i];
			glAttachShader(m_ProgramID, pShader->m_ShaderID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glLinkProgram(m_ProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (!success)
		{
			glGetProgramInfoLog(m_ProgramID, 512, NULL, infoLog);
			OpenGLGraphicsModule::LogGLError(glGetError());
			m_pOwner->GetEngine()->GetDebug().LogError(infoLog);
		}
	}
}
