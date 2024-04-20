#pragma once
#include "Resource.h"
#include "GraphicsEnums.h"

namespace Glory
{
    class FileData;
    class MaterialManager;
    class ShaderManager;

	class PipelineData : public Resource
	{
    public:
        PipelineData();
        virtual ~PipelineData();

        PipelineType Type() const;
        size_t ShaderCount() const;
        UUID ShaderID(size_t index) const;

        void SetPipelineType(PipelineType type);
        void AddShader(UUID shaderID);
        void RemoveShaderAt(size_t index);

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    protected:
        PipelineType m_Type = PT_Phong;
        std::vector<UUID> m_Shaders;
	};
}
