#version 450

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D textur;

float RTR(float hdrVal){
	float whiteSquared = 25.0;
	return hdrVal*(1.0 + hdrVal/whiteSquared)/(1.0 + hdrVal);
}

void main() {
	vec3 colour = texture(textur, v_texCoord).xyz;
	
	float L = dot(vec3(0.2126, 0.7152, 0.0722), colour);
	outColor = vec4(colour*RTR(L)/L, 1.0);
}
