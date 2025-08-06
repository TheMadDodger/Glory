#pragma once
#include <Pipeline.h>

#include <GL/glew.h>

namespace Glory
{
    /** @brief Compiled GPU pipeline for openGL */
    class OGLPipeline : public Pipeline
    {
    public:
        /** @brief Constructor */
        OGLPipeline(PipelineData* pPipelineData);
        /** @brief Destructor */
        virtual ~OGLPipeline();

        /** @brief Use this pipeline */
        void Use() override;
        virtual void UnUse() override;

        /** @brief Get the program ID of the pipeline */
        GLuint ProgramID();

    protected:
        /** @brief Initialize the pipeline by creating
         * a program and adding the shaders to it */
        void Initialize() override;

    private:
        GLuint m_ProgramID;
    };
}
