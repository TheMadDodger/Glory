#pragma once
#include <Resource.h>
#include <glm/glm.hpp>

#include <Reflection.h>

namespace Glory
{
    /** @brief Sound materia; */
    struct SoundMaterial
    {
        SoundMaterial():
            m_Absorption{ 0.2f, 0.2f, 0.2f }, m_Scattering{ 0.5f }, m_Transmission{0.1f, 0.1f, 0.1f} {}

        REFLECTABLE(SoundMaterial,
            (glm::vec3)(m_Absorption),
            (float)(m_Scattering),
            (glm::vec3)(m_Transmission)
        );
    };

    /** @brief Resource that can store a sound material */
    class SoundMaterialData : public Resource
    {
    public:
        GLORY_API SoundMaterialData();
        GLORY_API SoundMaterialData(SoundMaterial&& material);
        GLORY_API virtual ~SoundMaterialData();

        /** @brief Get the attached material */
        GLORY_API SoundMaterial& Material();
        /** @overload */
        GLORY_API const SoundMaterial& Material() const;

    private:
        SoundMaterial m_Material;
    };
}
