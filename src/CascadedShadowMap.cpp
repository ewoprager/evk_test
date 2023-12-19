#include "CascadedShadowMap.hpp"

// requires the camera projection and viewInverse matrices to be already set in the Main Global UBO
void UpdateCascades(Shared_Main::UBO_Global *const &mainUboGlobal, Shared_Shadow::UBO_Global *const &shadowUboGlobal){
	float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
	
	float minZ = Globals::cameraZNear;
	float maxZ = Globals::cameraZFar;
	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	const float smccInv = 1.0f/(float)SHADOW_MAP_CASCADE_COUNT;
	const float pThRootOfRatio = powf(ratio, smccInv);
	float minZpThRootOfRatioToTheI = minZ*pThRootOfRatio;
	for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; i++){
		float uniform = minZ + range*(float)(i + 1)*smccInv;
		float d = Globals::cascadeSplitLambda*(minZpThRootOfRatioToTheI - uniform) + uniform;
		cascadeSplits[i] = (d - minZ)/range;
		
		minZpThRootOfRatioToTheI *= pThRootOfRatio;
	}
	
	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; i++){
		float splitDist = cascadeSplits[i];

		vec3 frustumCorners[8] = {
			{-1.0f, 1.0f, 0.0f},
			{ 1.0f, 1.0f, 0.0f},
			{ 1.0f,-1.0f, 0.0f},
			{-1.0f,-1.0f, 0.0f},
			{-1.0f, 1.0f, 1.0f},
			{ 1.0f, 1.0f, 1.0f},
			{ 1.0f,-1.0f, 1.0f},
			{-1.0f,-1.0f, 1.0f}
		};

		// Project frustum corners into world space
		float viewProjection[4][4];
		M4x4_Multiply(mainUboGlobal->proj, mainUboGlobal->viewInv, viewProjection);
		float invCam[4][4];
		M4x4_Inverse(viewProjection, invCam);
		for(int i=0; i<8; i++){
			vec4 invCorner = M4x4_Multiply(invCam, frustumCorners[i] | 1.0f);
			frustumCorners[i] = invCorner.xyz()/invCorner.w;
		}

		for(int i=0; i<4; i++){
			vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}
	
		// Get frustum center
		vec3 frustumCenter = {0.0f, 0.0f, 0.0f};
		for(int i=0; i<8; i++) frustumCenter += frustumCorners[i];
		frustumCenter *= 0.125f;

		float radius = 0.0f;
		for(int i=0; i<8; i++){
			const float distance = sqrt((frustumCorners[i] - frustumCenter).SqMag());
			if(distance > radius) radius = distance;
		}
		radius = ceilf(radius*16.0f)/16.0f;

		float lightViewMatrix[4][4];
		M4x4_LookAt(frustumCenter - radius*Globals::lightDirection, frustumCenter, {0.0f, 0.0f, 1.0f}, lightViewMatrix);
		M4x4_Inverse(lightViewMatrix, lightViewMatrix);
		float lightOrthoMatrix[4][4];
		M4x4_Orthographic(-radius, radius, -radius, radius, 0.0f, 2.0f*radius, lightOrthoMatrix);
		
		// Store split distance and matrix in cascade
		mainUboGlobal->cascadeSplits[i] = (minZ + splitDist*range) * -1.0f;
		M4x4_Multiply(lightOrthoMatrix, lightViewMatrix, mainUboGlobal->lightMat[i]);
		memcpy(shadowUboGlobal->viewInvProj[i], mainUboGlobal->lightMat[i], 16*sizeof(float));
		
		lastSplitDist = cascadeSplits[i];
	}
}
