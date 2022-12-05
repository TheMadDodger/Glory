#pragma once
#include <imgui.h>

namespace Glory
{
    inline ImVec2 operator+(const ImVec2& v1, const ImVec2& v2)
    {
        ImVec2 result = ImVec2(v1);
        result.x += v2.x;
        result.y += v2.y;
        return result;
    }

    inline ImVec2 operator-(const ImVec2& v1, const ImVec2& v2)
    {
        ImVec2 result = ImVec2(v1);
        result.x -= v2.x;
        result.y -= v2.y;
        return result;
    }

    inline ImVec2 operator/(const ImVec2& v, float factor)
    {
        ImVec2 result = ImVec2(v);
        result.x /= factor;
        result.y /= factor;
        return result;
    }

    inline ImVec2 operator*(const ImVec2& v, float factor)
    {
        ImVec2 result = ImVec2(v);
        result.x *= factor;
        result.y *= factor;
        return result;
    }
}

namespace glm
{
    inline ImVec4 ToImVec4(const glm::vec4& vec)
    {
        return ImVec4(vec.x, vec.y, vec.z, vec.w);
    }
}
