#version 450

#define SHADOW_MAP_CASCADE_COUNT 4

layout(set = 0, binding = 0) uniform UBO_Global {
	mat4 viewInvProj[SHADOW_MAP_CASCADE_COUNT];
} ubo_g;

layout(set = 1, binding = 0) uniform UBO_PerObject {
	mat4 model;
	mat4 modelInvT;
} ubo_po;

layout(push_constant) uniform PCs {
	int cascadeLayer;
} pcs;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

void main() {
	gl_Position = ubo_g.viewInvProj[pcs.cascadeLayer] * ubo_po.model * vec4(a_position, 1.0);
}

