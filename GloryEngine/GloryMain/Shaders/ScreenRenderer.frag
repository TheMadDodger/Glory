#version 400

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D ScreenTexture;

void main()
{
	//out_Color = vec4(0.0, 1.0, 0.0, 1.0);

	float r = texture2D(ScreenTexture, Coord).x;

	out_Color = vec4(r, r, r, 1.0);
}