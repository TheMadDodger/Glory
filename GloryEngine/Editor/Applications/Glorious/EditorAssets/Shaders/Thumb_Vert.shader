#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 0) out vec2 Coord;

void main()
{
	Coord = vec2(clamp(in_Position.x, 0.0, 1.0), clamp(in_Position.y, 0.0, 1.0));
	gl_Position = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
}