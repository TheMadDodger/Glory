#pragma once

/*
* Need to merge the shader and shader data class into one class
* Which means the loader will have to ask the graphics module to make a shader object,
* and after filling in the data call an initialize function
*/

namespace Glory
{
	class Shader
	{
	public:
		Shader();
		virtual ~Shader();

	public:

	private:
	};
}
