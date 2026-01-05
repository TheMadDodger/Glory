#pragma once
#include "Module.h"

namespace Glory
{
    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

        /** @brief GraphicsModule type */
        const std::type_info& GetBaseModuleType() override;

        int GetLastDrawCalls();
        int GetLastVertexCount();
        int GetLastTriangleCount();

    protected:
        virtual void Initialize() = 0;
        virtual void Cleanup() = 0;
        virtual void OnBeginFrame() override;
        virtual void OnEndFrame() override;

        int m_CurrentDrawCalls;
        int m_LastDrawCalls;
        int m_LastVertices;
        int m_CurrentVertices;
        int m_LastTriangles;
        int m_CurrentTriangles;
    };
}