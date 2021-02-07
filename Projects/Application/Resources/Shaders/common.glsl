struct CameraData
{
	mat4 view;
	mat4 proj;
	vec4 position;
	float near;
	float far;
	float angle;
	float pad0;
};

// 64 bytes
struct Light
{
	// 16 bytes
	vec3 position; // either position or direction
	int type;

	// 16 bytes
	vec3 color;
	int amountOfLights;

	// 16 bytes
	float constantFactor;
	float linearFactor;
	float quadraticFactor;
	float cutoff;

	// 16 bytes
	vec4 pad;
};
