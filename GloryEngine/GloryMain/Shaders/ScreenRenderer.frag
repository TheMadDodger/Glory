#version 400

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
uniform sampler2D ScreenTexture;

void main()
{
	//out_Color = vec4(0.0, 1.0, 0.0, 1.0);

	out_Color = texture2D(ScreenTexture, Coord);
}