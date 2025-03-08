#include "Constraints.h"

#include <Hash.h>

namespace Glory
{
    std::vector<std::string> Constraints::m_Names = { "No constraint" };
    std::vector<std::string_view> Constraints::m_NamesView = { "No constraint" };
    std::vector<uint32_t> Constraints::m_Hashes = { 0 };
    std::vector<std::function<void(Constraint&, ConstraintAxis, const glm::vec2&, const glm::vec2&)>> Constraints::m_Callbacks = { NULL };

    void Constraints::AddConstraint(std::string&& name, std::function<void(Constraint&, ConstraintAxis, const glm::vec2&, const glm::vec2&)> callback)
    {
        m_Hashes.push_back(Hashing::Hash(name.c_str()));
        const size_t index = m_Names.size();
        m_Names.push_back(std::move(name));
        m_NamesView.push_back(m_Names[index]);
        m_Callbacks.push_back(callback);
    }

    const std::vector<std::string_view>& Constraints::Names()
    {
        return m_NamesView;
    }

    size_t Constraints::IndexOf(uint32_t type)
    {
        for (size_t i = 0; i < m_Hashes.size(); ++i)
        {
            if (m_Hashes[i] != type) continue;
            return i;
        }
        return 0;
    }

    size_t Constraints::IndexOf(std::string_view name)
    {
        for (size_t i = 0; i < m_Hashes.size(); ++i)
        {
            if (m_Names[i] != name) continue;
            return i;
        }
        return 0;
    }

    uint32_t Constraints::Type(size_t index)
    {
        return m_Hashes[index];
    }

    void Constraints::ProcessConstraint(ConstraintAxis axis, void* constraint, size_t handler, const glm::vec2& self, const glm::vec2& parent)
    {
        Constraint& constraintData = *reinterpret_cast<Constraint*>(constraint);
        m_Callbacks[handler](constraintData, axis, self, parent);
    }

    void RelativeConstraint(Constraint& constraint, ConstraintAxis axis, const glm::vec2&, const glm::vec2& parent)
    {
        switch (axis)
        {
        case Glory::X:
        case Glory::Width:
            constraint.m_FinalValue = constraint.m_Value <= 1.0f ? parent.x*constraint.m_Value : parent.x-constraint.m_Value;
            return;
        case Glory::Y:
        case Glory::Height:
            constraint.m_FinalValue = constraint.m_Value <= 1.0f ? parent.y*constraint.m_Value : parent.y-constraint.m_Value;
            return;
        default:
            break;
        }
        constraint.m_FinalValue = constraint.m_Value;
    }

    void CenterConstraint(Constraint& constraint, ConstraintAxis axis, const glm::vec2&, const glm::vec2& parent)
    {
        switch (axis)
        {
        case Glory::X:
            constraint.m_FinalValue = parent.x/2.0f + constraint.m_Value;
            return;
        case Glory::Y:
            constraint.m_FinalValue = parent.y/2.0f + constraint.m_Value;
            return;
        default:
            break;
        }
        constraint.m_FinalValue = constraint.m_Value;
    }

    void AspectConstraint(Constraint& constraint, ConstraintAxis axis, const glm::vec2& self, const glm::vec2&)
    {
        switch (axis)
        {
        case Glory::Width:
            constraint.m_FinalValue = self.y*constraint.m_Value;
            return;
        case Glory::Height:
            constraint.m_FinalValue = self.x*constraint.m_Value;
            return;
        default:
            break;
        }
        constraint.m_FinalValue = constraint.m_Value;
    }

    void Constraints::AddBuiltinConstraints()
    {
        ADD_CONSTRAINT_FUNC(RelativeConstraint);
        ADD_CONSTRAINT_FUNC(CenterConstraint);
        ADD_CONSTRAINT_FUNC(AspectConstraint);
    }
}
