#include "JoltShapeManager.h"
#include "Helpers.h"

namespace Glory
{
    PhysicsShape* Glory::JoltShapeManager::CreateShape_Internal(const Shape& shape, const UUID uuid)
    {
        JPH::Shape* pShape = GetJPHShape(shape);
        return (PhysicsShape*)pShape;
    }
}
