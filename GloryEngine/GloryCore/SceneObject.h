#pragma once
#include "Object.h"
#include "Layer.h"

namespace Glory
{
    class SceneObject : public Object
    {
    public:
        SceneObject();
        SceneObject(const std::string& name);
        SceneObject(const std::string& name, UUID uuid);
        virtual ~SceneObject();

        virtual SceneObject* GetParent() = 0;
        virtual void SetParent(SceneObject* pParent) = 0;

    public:
        const std::string& Name();
        void SetName(const std::string& name);

    protected:
        virtual void Initialize() = 0;

    private:
        friend class GScene;
        std::string m_Name;
    };
}
