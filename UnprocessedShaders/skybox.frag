#version 450

#define FOG_MAX 0.003
#define FOG_DECREASE 0.004
#define AMBIENT 0.3

layout(binding = 0) uniform UBO_Global {
	mat4 viewInv;
	mat4 proj;
	vec4 cameraPosition;
} ubo_g;

layout(binding = 1) uniform samplerCube u_cubemap;

layout(location = 0) in vec3 v_UVW;

layout(location = 0) out vec4 outColor;

void main(){
	vec4 fogColour = vec4(vec3(AMBIENT), 1.0);
	
	if(v_UVW.z < 0.0) outColor = fogColour;
	else {
		outColor = texture(u_cubemap, v_UVW);
		
		float lh = length(v_UVW.xy);
		float fogginessSummedVertically = FOG_MAX*exp(-ubo_g.cameraPosition.z*FOG_DECREASE)/FOG_DECREASE;
		float fogginessSummedTotal = fogginessSummedVertically*sqrt(lh*lh + v_UVW.z*v_UVW.z)/v_UVW.z;
		outColor = mix(fogColour, outColor, exp(-fogginessSummedTotal));
	}
}
