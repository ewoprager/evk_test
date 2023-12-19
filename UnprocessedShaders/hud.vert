#version 450

// currently all stuff is bound to bottom right corner
// positions are such that the bottom right of the shape is at (0, 0), and the scale is pixels

layout(binding = 1) uniform UniformBufferObject {
	float extentWidth; // pixels
	float extentHeight; // pixels
	float gap; // pixels
} ubo;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_colour;
layout(location = 2) in vec2 a_texCoord;

layout(location = 0) out vec4 v_colour;
layout(location = 1) out vec2 v_texCoord;

void main() {
	gl_Position = vec4(2.0*(a_position - vec2(ubo.gap, ubo.gap))/vec2(ubo.extentWidth, ubo.extentHeight) + vec2(1.0, 1.0), 0.0, 1.0);
	v_colour = a_colour;
	v_texCoord = a_texCoord;
}
