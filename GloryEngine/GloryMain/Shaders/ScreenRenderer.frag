#version 400

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D Color;
layout (binding = 1) uniform sampler2D Position;
layout (binding = 2) uniform sampler2D Normal;
layout (binding = 3) uniform sampler2D Depth;

//uniform vec3 eyeDirection;

uniform float zNear;
uniform float zFar;
uniform uint tileSizeInPx; // How many pixels a rectangular cluster takes in x and y
uniform uvec3 numClusters; // The fixed number of clusters in x y and z axes
uniform vec2 screenResolution;

vec3 light = vec3(-0.707, 0.0, 0.707);
vec3 specularColor = vec3(1.0, 1.0, 1.0);

uint getClusterIndex(vec3 pixelCoord);
uint getDepthSlice(float z);
float linearDepth(float depthSample);

const vec3 depthSliceColors[8] = vec3[8](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 1.0, 0.0),
	vec3(0.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(0.5, 0.5, 0.5)
);

void main()
{
	//out_Color = vec4(0.0, 1.0, 0.0, 1.0);

	vec3 lightVec = normalize(light);
	vec3 normal = texture2D(Normal, Coord).xyz;
	float diffuse = dot(-lightVec, normal);

	//vec3 lightReflected = reflect(lightVec, normal);
	//float specular = pow(dot(-lightReflected, eyeDirection), 5);

	vec2 pixelID = Coord * screenResolution;
	float depth = texture2D(Depth, Coord).r;
	uint clusterID = getClusterIndex(vec3(pixelID.xy, depth));

	uint clusterColorIndex = uint(mod(clusterID, 5));
	uint totalClusters = numClusters.x * numClusters.y * numClusters.z;
	float clusterFrac = float(clusterColorIndex) / float(totalClusters);

	uint clusterZVal = getDepthSlice(depth);
	vec3 clusterColor = depthSliceColors[uint(mod(clusterColorIndex, 5.0))];

	vec4 color = texture2D(Color, Coord);
	out_Color = vec4(color.xyz * diffuse + clusterColor, 1.0);
}

uint getClusterIndex(vec3 pixelCoord)
{
    uint clusterZVal = getDepthSlice(pixelCoord.z);

    uvec3 clusters = uvec3(uvec2(pixelCoord.xy / tileSizeInPx), clusterZVal);
    uint clusterIndex = clusters.x +
        numClusters.x * clusters.y +
        (numClusters.x * numClusters.y) * clusters.z;
    return clusterIndex;
}

uint getDepthSlice(float depth)
{
	float scale = 1.0;
	float bias = 0.0;
	return uint(max(log2(linearDepth(depth)) * scale + bias, 0.0));
}

float linearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}
