#include "JoltShapeManager.h"
#include "Helpers.h"

namespace Glory
{
    PhysicsShape* Glory::JoltShapeManager::CreateShape_Internal(const Shape& shape, const UUID uuid)
    {
        JPH::Shape* pShape = GetJPHShape(shape);
        return (PhysicsShape*)pShape;
    }

    void JoltShapeManager::DestroyShape_Internal(ShapeData& shapeData)
    {
        if (!shapeData.m_Owned) return;
        JPH::Shape* pShape = (JPH::Shape*)shapeData.m_pShape;
        delete pShape;
    }
}
