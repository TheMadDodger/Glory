#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 6) buffer ssaoSettings
{
    int AOEnabled;
    int Dirty;
    float SampleRadius;
    float SampleBias;
    int KernelSize;
    int BlurType;
    int BlurSize;
    float Separation;
    int BinsSize;
};

/* @todo: Move these to a helper file */
#define MAX_SIZE        4
#define MAX_KERNEL_SIZE ((MAX_SIZE * 2 + 1) * (MAX_SIZE * 2 + 1))
#define MAX_BINS_SIZE   100

vec4 BoxBlur(sampler2D tex, int blurSize, float seperation)
{
    vec2 texSize = textureSize(tex, 0).xy;
	vec2 texCoord = gl_FragCoord.xy / texSize;

    if (blurSize <= 0)
    {
        return texture2D(tex, texCoord);
    }
    seperation = max(seperation, 1.0);

    vec3 color = vec3(0.0);
	for(int x = -blurSize; x <= blurSize; ++x)
	{
	    for(int y = -blurSize; y <= blurSize; ++y)
	    {
	        color += texture2D(tex, (gl_FragCoord.xy + vec2(x, y)*seperation)/texSize).xyz;
	    }
	}
	color /= pow(blurSize*2.0 + 1.0, 2.0);
	return vec4(vec3(color), 1.0);
}

vec4 MedianBlur(sampler2D tex, int blurSize, int binsSize)
{
    vec2 texSize = textureSize(tex, 0).xy;
    vec2 texCoord = gl_FragCoord.xy / texSize;

    if (blurSize <= 0)
    {
        return texture2D(tex, texCoord);
    }
	
    if (blurSize > MAX_SIZE)
    {
        blurSize = MAX_SIZE;
    }
    int kernelSize = int(pow(blurSize * 2 + 1, 2));
    binsSize = clamp(binsSize, 1, MAX_BINS_SIZE);

    int i = 0;
    int j = 0;
    int count = 0;
    int binIndex = 0;

    vec4 colors[MAX_KERNEL_SIZE];
    float bins[MAX_BINS_SIZE];
    int binIndexes[colors.length()];

    float total = 0;
    float limit = floor(float(kernelSize) / 2) + 1;

    float value = 0;
    vec3 valueRatios = vec3(0.3, 0.59, 0.11);

    for (i = -blurSize; i <= blurSize; ++i)
    {
        for (j = -blurSize; j <= blurSize; ++j)
        {
            colors[count] =

            texture2D(tex, ( gl_FragCoord.
            xy + vec2(i, j)) / texSize);
            count += 1;
        }
    }

    for (i = 0; i < binsSize; ++i)
    {
        bins[i] = 0;
    }

    for (i = 0; i < kernelSize; ++i)
    {
        value = dot(colors[i].rgb, valueRatios);
        binIndex = int(floor(value * binsSize));
        binIndex = clamp(binIndex, 0, binsSize - 1);
        bins[binIndex] += 1;
        binIndexes[i] = binIndex;
    }

    binIndex = 0;

    for (i = 0; i < binsSize; ++i)
    {
        total += bins[i];
        if (total >= limit)
        {
            binIndex = i;
            break;
        }
    }

    vec4 color = colors[0];

    for (i = 0; i < kernelSize; ++i)
    {
        if (binIndexes[i] == binIndex)
        {
            color = colors[i];
            break;
        }
    }
    return color;
}

in vec2 Coord;
layout(location = 3) out vec4 out_Color;
layout (binding = 0) uniform sampler2D AO;

void main()
{
	int blurSize = 4;
	int binsSize = 5;

    if(BlurType == 0)
    {
        out_Color = BoxBlur(AO, BlurSize, Separation);
    }
    else if(BlurType == 1)
    {
	    out_Color = MedianBlur(AO, BlurSize, BinsSize);
    }
}
