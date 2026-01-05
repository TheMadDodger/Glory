#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (set = 0, binding = 0) uniform sampler2D Color;
layout (set = 1, binding = 1) uniform sampler2D AO;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
    float Magnitude;
    float Contrast;

} Constants;

void main()
{
    float ssao = Constants.Magnitude*pow(texture(AO, Coord).x, Constants.Contrast);
    vec3 color = texture(Color, Coord).xyz;

	out_Color = vec4(color*ssao, 1.0);
}
