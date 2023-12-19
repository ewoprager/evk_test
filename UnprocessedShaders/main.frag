#version 450

#define PNGS_N 4
#define SHADOW_MAP_CASCADE_COUNT 4
#define FOG_MAX 0.003
#define FOG_DECREASE 0.004
#define LIGHT_STRENGTH 3.0
#define AMBIENT 0.3

layout(set = 0, binding = 0) uniform UBO_Global {
	mat4 lightMat[SHADOW_MAP_CASCADE_COUNT];
	mat4 viewInv;
	mat4 proj;
	vec4 lightColour;
	vec4 cascadeSplits;
	vec4 lightDir;
	vec4 cameraPosition;
} ubo_g;

layout(push_constant) uniform PushConstants {
	layout(offset = 16) vec4 colourMult;
	vec4 specular;
	float shininess;
	float specularFactor;
	int textureID;
} pcs;

layout(set = 0, binding = 1) uniform sampler texSampler;
layout(set = 0, binding = 2) uniform texture2D textur[PNGS_N]; // ! must equal the number of different textures (`IMGS_N`)

layout(set = 0, binding = 3) uniform sampler2DArray shadowMap;

layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec2 v_texCoord;
layout(location = 2) in vec3 v_surfaceToCamera;
layout(location = 3) in vec3 v_viewPos;
layout(location = 4) in vec3 v_position;

layout(location = 0) out vec4 outColor;

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);
float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex){
	float shadow = 1.0;
	float bias = 0.005;

	if(shadowCoord.z > -1.0 && shadowCoord.z < 1.0){
		float dist = texture(shadowMap, vec3(shadowCoord.xy + offset, cascadeIndex)).r;
		if(shadowCoord.w > 0 && dist < shadowCoord.z - bias){
			shadow = 0;
		}
	}
	return shadow;
}
float filterPCF(vec4 shadowCoord, uint cascadeIndex){
	ivec2 texDim = textureSize(shadowMap, 0).xy;
	float scale = 2.0;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int halfRange = 2;
	
	for(int x=-halfRange; x<=halfRange; x++){
		for (int y=-halfRange; y<=halfRange; y++){
			shadowFactor += textureProj(shadowCoord, vec2(dx*x, dy*y), cascadeIndex);
		}
	}
	
	int range = halfRange + halfRange + 1;
	return shadowFactor / float(range*range);
}
vec2 ilumination(uint cascadeIndex, float l, float h, float m){
	if(l > 0.0){
		vec4 shadowCoord = biasMat * ubo_g.lightMat[cascadeIndex] * vec4(v_position, 1.0);
		float shade = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex);
		return shade*LIGHT_STRENGTH*vec2(l, pow(max(0.0, h), m));
	} else return vec2(0.0);
}

void main(){
	// Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for(uint i=0; i<SHADOW_MAP_CASCADE_COUNT - 1; i++){
		if(v_viewPos.z < ubo_g.cascadeSplits[i]){
			cascadeIndex = i + 1;
		}
	}
	
	vec4 diffuseColour = texture(sampler2D(textur[pcs.textureID], texSampler), v_texCoord);
	vec3 a_normal = normalize(v_normal);
	vec3 surfaceToLight = normalize(-ubo_g.lightDir.xyz);
	vec3 surfaceToCamera = normalize(v_surfaceToCamera);
	vec3 halfVector = normalize(surfaceToLight + surfaceToCamera);
	vec2 ilu = ilumination(cascadeIndex, dot(a_normal, surfaceToLight), dot(a_normal, halfVector), pcs.shininess);
	outColor = vec4((ubo_g.lightColour * (diffuseColour * (ilu.x + AMBIENT) * pcs.colourMult + pcs.specular * ilu.y * pcs.specularFactor)).rgb, diffuseColour.a);
	
	vec4 fogColour = vec4(vec3(AMBIENT), 1.0);
	float fogginessSummedVertically = FOG_MAX*abs(exp(-ubo_g.cameraPosition.z*FOG_DECREASE) - exp(-v_position.z*FOG_DECREASE))/FOG_DECREASE;
	float fogginessSummedTotal = fogginessSummedVertically*-v_viewPos.z/abs(ubo_g.cameraPosition.z - v_position.z);
	outColor = mix(fogColour, outColor, exp(-fogginessSummedTotal));
}
