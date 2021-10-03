#pragma once
#include "GPUResource.h"

/* Material should hold information about all the shaders linked to it
* This involves:
* - Input and output of each shader
* These are defined using the following attribute like line
* [INPUT("Position")]
* [OUTPUT("Color")]
* - Shader properties like variables uniform buffers etc
* The shader file has all of these defined in the form of attributes like for example
* [PROPERTY("Intensity")]
* This means shader files will have to be pre processed before they are compiled.
* For compilation the material will ask the graphics module to retreive the compiled shader.
* Depending on the module it would check if the shader was already compiled somewhere,
* in case of Vulkan it would have been compiled to a spirv file before,
* and returns the compiled version of the shader.
* If the shader hadn't been compiled yet it will compile the shader source,
* the provided shader source needs to have been preprocessed to get rid of all attributes
* and include directives as the compiler won't be able to process those.
* 
* Possibly, materials could also be stored in a text file, that defines all shaders it uses.
* This file could also store the input output and property data,
* but for this it will also need to keep track of the version of each shader
* in order to process these again when the shader is changed.
*/

namespace Glory
{
	class Material : public GPUResource
	{
	};
}
