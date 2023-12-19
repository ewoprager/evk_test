#version 450

layout(binding = 0) uniform UBO_Global {
	mat4 viewInv;
	mat4 proj;
	vec4 cameraPosition;
} ubo_g;

layout(location = 0) in vec3 a_position;

layout(location = 0) out vec3 v_UVW;

void main(){
	v_UVW = a_position;
	v_UVW.xy *= -1.0;
	
	gl_Position = ubo_g.proj * ubo_g.viewInv * vec4(a_position, 1.0);
}

