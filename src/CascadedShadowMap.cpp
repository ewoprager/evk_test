#include "CascadedShadowMap.hpp"

// requires the camera projection and viewInverse matrices to be already set in the Main Global UBO
void UpdateCascades(PipelineMain::UBO_Global *mainUboGlobal, PipelineShadow::UBO_Global *shadowUboGlobal){
	float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
	
	float minZ = Globals::cameraZNear;
	float maxZ = Globals::cameraZFar;
	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	const float smccInv = 1.0f/(float)SHADOW_MAP_CASCADE_COUNT;
	const float pThRootOfRatio = powf(ratio, smccInv);
	float minZpThRootOfRatioToTheI = minZ * pThRootOfRatio;
	for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; i++){
		float uniform = minZ + range * float(i + 1) * smccInv;
		float d = Globals::cascadeSplitLambda*(minZpThRootOfRatioToTheI - uniform) + uniform;
		cascadeSplits[i] = (d - minZ) / range;
		
		minZpThRootOfRatioToTheI *= pThRootOfRatio;
	}
	
	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; ++i){
		float splitDist = cascadeSplits[i];

		vec<3> frustumCorners[8] = {
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
		mat<4, 4> viewProjection = mainUboGlobal->proj & mainUboGlobal->viewInv;
		mat<4, 4> invCam = viewProjection.Inverted();
		for(int i=0; i<8; i++){
			vec<4> invCorner = invCam & (frustumCorners[i] | 1.0f);
			frustumCorners[i] = (vec<3>){invCorner.x, invCorner.y, invCorner.z} / invCorner.w;
		}

		for(int i=0; i<4; i++){
			vec<3> dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}
	
		// Get frustum center
		vec<3> frustumCenter {0.0f, 0.0f, 0.0f};
		for(int i=0; i<8; i++) frustumCenter += frustumCorners[i];
		frustumCenter *= 0.125f;

		float radius = 0.0f;
		for(int i=0; i<8; i++){
			const float distance = sqrt((frustumCorners[i] - frustumCenter).SqMag());
			if(distance > radius) radius = distance;
		}
		radius = ceilf(radius*16.0f)/16.0f;

		mat<4, 4> lightViewMatrix = mat<4, 4>::LookAt(frustumCenter - radius*Globals::lightDirection, frustumCenter, {0.0f, 0.0f, 1.0f}).Inverted();
		mat<4, 4> lightOrthoMatrix = mat<4, 4>::OrthographicProjection(-radius, radius, -radius, radius, 0.0f, 2.0f*radius);
		
		// Store split distance and matrix in cascade
		mainUboGlobal->cascadeSplits[i] = (minZ + splitDist * range) * -1.0f;
		mainUboGlobal->lightMat[i] = lightOrthoMatrix & lightViewMatrix;
		shadowUboGlobal->viewInvProj[i] = mainUboGlobal->lightMat[i];
		
		lastSplitDist = cascadeSplits[i];
	}
}
