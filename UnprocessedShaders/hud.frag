#version 450

layout(location = 0) in vec4 v_colour;
layout(location = 1) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler texSampler;
layout(binding = 2) uniform texture2D textur;

void main() {
	outColor = texture(sampler2D(textur, texSampler), v_texCoord);
}
