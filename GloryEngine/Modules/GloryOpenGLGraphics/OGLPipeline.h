#pragma once
#include <Pipeline.h>

#include <GL/glew.h>

namespace Glory
{
    class OGLPipeline : public Pipeline
    {
    public:
        OGLPipeline(PipelineData* pPipelineData);
        virtual ~OGLPipeline();

        void Use() override;

        GLuint ProgramID();

    protected:
        void Initialize() override;

    private:
        GLuint m_ProgramID;
    };
}
