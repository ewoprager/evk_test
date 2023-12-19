#version 450

#define SHADOW_MAP_CASCADE_COUNT 4

layout(set = 0, binding = 0) uniform UBO_Global {
	mat4 lightMat[SHADOW_MAP_CASCADE_COUNT];
	mat4 viewInv;
	mat4 proj;
	vec4 lightColour;
	vec4 cascadeSplits;
	vec4 lightDir;
	vec4 cameraPosition;
} ubo_g;

layout(push_constant) uniform PCs {
	int placeHolder;
} pcs;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in mat4 a_model;
layout(location = 7) in mat4 a_modelInvT;

layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec2 v_texCoord;
layout(location = 2) out vec3 v_surfaceToCamera;
layout(location = 3) out vec3 v_viewPos;
layout(location = 4) out vec3 v_position;

void main() {
	vec4 positionWorld = a_model * vec4(a_position, 1.0);
	vec4 positionView = ubo_g.viewInv * positionWorld;
	v_normal = (a_modelInvT * vec4(a_normal, 0.0)).xyz;
	v_texCoord = a_texCoord;
	v_surfaceToCamera = ubo_g.cameraPosition.xyz - positionWorld.xyz;
	v_viewPos = positionView.xyz;
	v_position = positionWorld.xyz;
	
	gl_Position = ubo_g.proj * positionView;
}
