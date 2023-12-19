#include "Pipelines.hpp"

extern int textureImageIndexArray[PNGS_N];

EVK::Interface NewBuildPipelines(const EVK::Devices &devices, EVK::ImageBlueprint **const &imageBlueprintPtrs, int shadowMapImageIndex, int skyboxImageIndex, int finalColourImageIndex, int finalDepthImageIndex){
	
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	
	//If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them. This is useful in some special cases like shadow maps.
	 
	//Using this requires enabling a GPU feature.
	
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	
	//The polygonMode determines how fragments are generated for geometry. The following modes are available:
	 
	//VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	//VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
	//VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
	 
	//Using any mode other than fill requires enabling a GPU feature.
	
	rasterizer.lineWidth = 1.0f;
	//The lineWidth member is straightforward, it describes the thickness of lines in terms of number of fragments. The maximum line width that is supported depends on the hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.
	
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//The cullMode variable determines the type of face culling to use. You can disable culling, cull the front faces, cull the back faces or both. The frontFace variable specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise.
	
	//rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	
	
	// ----- Multisampling behaviour -----
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	//#ifdef MSAA
	//multisampling.rasterizationSamples = device.GetMSAASamples();
	//#else
	//multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//#endif
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	// This is how to achieve anti-aliasing.
	// Enabling it requires enabling a GPU feature
	
	
	// ----- Colour blending behaviour -----
	VkPipelineColorBlendStateCreateInfo colourBlending{};
	VkPipelineColorBlendAttachmentState colourBlendAttachment{};
	colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourBlendAttachment.blendEnable = VK_TRUE;
	colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	
	colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlending.logicOpEnable = VK_FALSE;
	colourBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	//colourBlending.attachmentCount = 1;
	colourBlending.pAttachments = &colourBlendAttachment;
	colourBlending.blendConstants[0] = 0.0f; // Optional
	colourBlending.blendConstants[1] = 0.0f; // Optional
	colourBlending.blendConstants[2] = 0.0f; // Optional
	colourBlending.blendConstants[3] = 0.0f; // Optional
	
	
	// ----- Depth stencil behaviour -----
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	//depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	
	// ----- Dynamic state -----
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkDynamicState dynamicStates[3] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
	};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	VkPipelineShaderStageCreateInfo shaderStages[2];
	// vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";
	//shaderStages[0].module = ...;
	// this is for constants to use in the shader:
	shaderStages[0].pSpecializationInfo = nullptr;
	// fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";
	//shaderStages[1].module = ...;
	// this is for constants to use in the shader:
	shaderStages[1].pSpecializationInfo = nullptr;
	
	
	
	const int mainPushConstantRangesN = 2;
	VkPushConstantRange mainPushConstantRanges[mainPushConstantRangesN] = {
		{
			VK_SHADER_STAGE_VERTEX_BIT,				// shader stage
			0,										// offset
			sizeof(Shared_Main::PushConstants_Vert)	// size
		},
		{
			VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof(Shared_Main::PushConstants_Vert),
			sizeof(Shared_Main::PushConstants_Frag)
		}
	};
	VkPushConstantRange shadowPushConstantRange = {
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Shared_Shadow::PushConstants_Vert)
	};
	
	//int textureImageIndexArray[PNGS_N]; for(int i=0; i<PNGS_N; i++) textureImageIndexArray[i] = i + SHADOW_MAPS_N + SKY_BOXES_N;
	
	int uboMainGlobalIndex = (int)UBO::mainGlobal;
	int samplerMainIndex = (int)Sampler::main;
	int samplerShadowIndex = (int)Sampler::shadow;
	int uboShadowIndex = (int)UBO::shadow;
	int uboHudIndex = (int)UBO::hud;
	int uboMainPerObjectIndex = (int)UBO::mainPerObject;
	int uboSkyboxIndex = (int)UBO::skybox;
	int skyboxSamplerIndex = (int)Sampler::cube;
	int histogramUboIndex = (int)UBO::histogram;
	int histogramSboIndex = (int)SBO::histogram;
	
	EVK::DescriptorBlueprint mainDescriptorSet0DescriptorBlueprints[4] = {
		{
			EVK::DescriptorType::UBO,										// type
			Shared_Main::UBO_Global::binding,							// binding
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,	// shader stage(s)
			1,															// count
			&uboMainGlobalIndex											// indices
		},
		{
			EVK::DescriptorType::textureImage,
			Shared_Main::textureImagesBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			PNGS_N,
			textureImageIndexArray
		},
		{
			EVK::DescriptorType::textureSampler,
			Shared_Main::textureSamplerBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&samplerMainIndex
		},
		{
			EVK::DescriptorType::combinedImageSampler,
			Shared_Main::shadowMapBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&shadowMapImageIndex,
			&samplerShadowIndex
		}
	};
	EVK::DescriptorSetBlueprint mainDescriptorSet0Build = {4, mainDescriptorSet0DescriptorBlueprints};
	EVK::DescriptorBlueprint shadowDescriptorSetDescriptorBuild[1] = {
		{
			EVK::DescriptorType::UBO,
			Shared_Shadow::UBO_Global::binding,
			VK_SHADER_STAGE_VERTEX_BIT,
			1,
			&uboShadowIndex
		}
	};
	EVK::DescriptorSetBlueprint shadowDescriptorSet0Build = {1, shadowDescriptorSetDescriptorBuild};
	EVK::DescriptorBlueprint onceDescriptorSetDescriptorBlueprints = {
		EVK::DescriptorType::UBO,
		Pipeline_MainOnce::perObjectUboBinding,
		VK_SHADER_STAGE_VERTEX_BIT,
		1,
		&uboMainPerObjectIndex
	};
	
	// main instanced
	EVK::GraphicsPipelineBlueprint pbMainInstanced = {};
	pbMainInstanced.pipelineBlueprint.descriptorSetsN = 1;
	EVK::DescriptorSetBlueprint mainInstancedDescriptorSetBlueprints[1] = {mainDescriptorSet0Build};
	pbMainInstanced.pipelineBlueprint.descriptorSetBlueprints = mainInstancedDescriptorSetBlueprints;
	pbMainInstanced.pipelineBlueprint.pushConstantRangesN = mainPushConstantRangesN;
	pbMainInstanced.pipelineBlueprint.pushConstantRanges = mainPushConstantRanges;
	pbMainInstanced.stageCount = 2;
	VkPipelineShaderStageCreateInfo mainInstancedShaderStages[2];
	memcpy(mainInstancedShaderStages, shaderStages, 2*sizeof(VkPipelineShaderStageCreateInfo));
	mainInstancedShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertMainInstanced.spv");
	mainInstancedShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragMain.spv");
	pbMainInstanced.shaderStageCIs = mainInstancedShaderStages;
	vertexInputInfo = Pipeline_MainInstanced::Attributes::stateCI;
	pbMainInstanced.vertexInputStateCI = vertexInputInfo;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	pbMainInstanced.rasterisationStateCI = rasterizer;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	pbMainInstanced.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 1;
	pbMainInstanced.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	pbMainInstanced.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 2;
	pbMainInstanced.dynamicStateCI = dynamicState;
	pbMainInstanced.bufferedRenderPassIndex = (int)BRP::finall;
	pbMainInstanced.layeredBufferedRenderPassIndex = -1;
	
	// shadow instanced
	EVK::GraphicsPipelineBlueprint pbShadowInstanced = {};
	pbShadowInstanced.pipelineBlueprint.descriptorSetsN = 1;
	EVK::DescriptorSetBlueprint shadowInstancedDescriptorSetBlueprints[1] = {shadowDescriptorSet0Build};
	pbShadowInstanced.pipelineBlueprint.descriptorSetBlueprints = shadowInstancedDescriptorSetBlueprints;
	pbShadowInstanced.pipelineBlueprint.pushConstantRangesN = 1;
	pbShadowInstanced.pipelineBlueprint.pushConstantRanges = &shadowPushConstantRange;
	pbShadowInstanced.stageCount = 1;
	VkPipelineShaderStageCreateInfo shadowInstancedShaderStage = shaderStages[0];
	shadowInstancedShaderStage.module = devices.CreateShaderModule("../Resources/Shaders/vertShadowInstanced.spv");
	pbShadowInstanced.shaderStageCIs = &shadowInstancedShaderStage;
	pbShadowInstanced.vertexInputStateCI = vertexInputInfo; // same as mainInstanced
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	pbShadowInstanced.rasterisationStateCI = rasterizer;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pbShadowInstanced.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 0;
	pbShadowInstanced.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pbShadowInstanced.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 3;
	pbShadowInstanced.dynamicStateCI = dynamicState;
	pbShadowInstanced.bufferedRenderPassIndex = -1;
	pbShadowInstanced.layeredBufferedRenderPassIndex = (int)LBRP::shadow;
	
	
	// main once
	EVK::GraphicsPipelineBlueprint pbMainOnce = {};
	pbMainOnce.pipelineBlueprint.descriptorSetsN = 2;
	EVK::DescriptorSetBlueprint mainOnceDescriptorSetBlueprints[2];
	mainOnceDescriptorSetBlueprints[0] = mainDescriptorSet0Build;
	mainOnceDescriptorSetBlueprints[1] = {1, &onceDescriptorSetDescriptorBlueprints};
	pbMainOnce.pipelineBlueprint.descriptorSetBlueprints = mainOnceDescriptorSetBlueprints;
	pbMainOnce.pipelineBlueprint.pushConstantRangesN = mainPushConstantRangesN;
	pbMainOnce.pipelineBlueprint.pushConstantRanges = mainPushConstantRanges;
	pbMainOnce.stageCount = 2;
	VkPipelineShaderStageCreateInfo mainOnceShaderStages[2];
	memcpy(mainOnceShaderStages, shaderStages, 2*sizeof(VkPipelineShaderStageCreateInfo));
	mainOnceShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertMainOnce.spv");
	mainOnceShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragMain.spv");
	pbMainOnce.shaderStageCIs = mainOnceShaderStages;
	vertexInputInfo = Pipeline_MainOnce::Attributes::stateCI;
	pbMainOnce.vertexInputStateCI = vertexInputInfo;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	pbMainOnce.rasterisationStateCI = rasterizer;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	pbMainOnce.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 1;
	pbMainOnce.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	pbMainOnce.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 2;
	pbMainOnce.dynamicStateCI = dynamicState;
	pbMainOnce.bufferedRenderPassIndex = (int)BRP::finall;
	pbMainOnce.layeredBufferedRenderPassIndex = -1;
	
	// shadow once
	EVK::GraphicsPipelineBlueprint pbShadowOnce = {};
	pbShadowOnce.pipelineBlueprint.descriptorSetsN = 2;
	EVK::DescriptorSetBlueprint shadowOnceDescriptorSetBlueprints[2];
	shadowOnceDescriptorSetBlueprints[0] = shadowDescriptorSet0Build;
	shadowOnceDescriptorSetBlueprints[1] = {1, &onceDescriptorSetDescriptorBlueprints};
	pbShadowOnce.pipelineBlueprint.descriptorSetBlueprints = shadowOnceDescriptorSetBlueprints;
	pbShadowOnce.pipelineBlueprint.pushConstantRangesN = 1;
	pbShadowOnce.pipelineBlueprint.pushConstantRanges = &shadowPushConstantRange;
	pbShadowOnce.stageCount = 1;
	VkPipelineShaderStageCreateInfo shadowOnceShaderStage = shaderStages[0];
	shadowOnceShaderStage.module = devices.CreateShaderModule("../Resources/Shaders/vertShadowOnce.spv");
	pbShadowOnce.shaderStageCIs = &shadowOnceShaderStage;
	pbShadowOnce.vertexInputStateCI = vertexInputInfo; // same as mainOnce
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	pbShadowOnce.rasterisationStateCI = rasterizer;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pbShadowOnce.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 0;
	pbShadowOnce.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pbShadowOnce.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 3;
	pbShadowOnce.dynamicStateCI = dynamicState;
	pbShadowOnce.bufferedRenderPassIndex = -1;
	pbShadowOnce.layeredBufferedRenderPassIndex = (int)LBRP::shadow;
	
	
	// hud
	EVK::GraphicsPipelineBlueprint pbHud = {};
	int hudImageIndex = 4;
	pbHud.pipelineBlueprint.descriptorSetsN = 1;
	EVK::DescriptorBlueprint hudDescriptorSetDescriptorBlueprints[3] = {
		{
			EVK::DescriptorType::UBO,
			Pipeline_Hud::UBO::binding,
			VK_SHADER_STAGE_VERTEX_BIT,
			1,
			&uboHudIndex
		},
		{
			EVK::DescriptorType::textureSampler,
			Pipeline_Hud::textureSamplerBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&samplerMainIndex
		},
		{
			EVK::DescriptorType::textureImage,
			Pipeline_Hud::textureImagesBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&hudImageIndex
		}
	};
	EVK::DescriptorSetBlueprint hudDescriptorSetBlueprints[1] = {{3, hudDescriptorSetDescriptorBlueprints}};
	pbHud.pipelineBlueprint.descriptorSetBlueprints = hudDescriptorSetBlueprints;
	pbHud.pipelineBlueprint.pushConstantRangesN = 0;
	pbHud.stageCount = 2;
	VkPipelineShaderStageCreateInfo hudShaderStages[2];
	memcpy(hudShaderStages, shaderStages, 2*sizeof(VkPipelineShaderStageCreateInfo));
	hudShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertHud.spv");
	hudShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragHud.spv");
	pbHud.shaderStageCIs = hudShaderStages;
	vertexInputInfo = Pipeline_Hud::Attributes::stateCI;
	pbHud.vertexInputStateCI = vertexInputInfo;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
	pbHud.rasterisationStateCI = rasterizer;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	pbHud.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 1;
	pbHud.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	pbHud.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 2;
	pbHud.dynamicStateCI = dynamicState;
	pbHud.bufferedRenderPassIndex = (int)BRP::finall;
	pbHud.layeredBufferedRenderPassIndex = -1;
	
	
	// skybox
	EVK::GraphicsPipelineBlueprint pbSkybox = {};
	pbSkybox.pipelineBlueprint.descriptorSetsN = 1;
	EVK::DescriptorBlueprint skyboxDescriptorSetDescriptorBlueprints[2] = {
		{
			EVK::DescriptorType::UBO,
			Pipeline_Skybox::UBO_Global::binding,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&uboSkyboxIndex
		},
		{
			EVK::DescriptorType::combinedImageSampler,
			Pipeline_Skybox::cubemapBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&skyboxImageIndex,
			&skyboxSamplerIndex
		}
	};
	EVK::DescriptorSetBlueprint skyboxDescriptorSetBlueprints[1] = {{2, skyboxDescriptorSetDescriptorBlueprints}};
	pbSkybox.pipelineBlueprint.descriptorSetBlueprints = skyboxDescriptorSetBlueprints;
	pbSkybox.pipelineBlueprint.pushConstantRangesN = 0;
	pbSkybox.stageCount = 2;
	VkPipelineShaderStageCreateInfo skyboxShaderStages[2];
	memcpy(skyboxShaderStages, shaderStages, 2*sizeof(VkPipelineShaderStageCreateInfo));
	skyboxShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertSkybox.spv");
	skyboxShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragSkybox.spv");
	pbSkybox.shaderStageCIs = skyboxShaderStages;
	vertexInputInfo = Pipeline_Skybox::Attributes::stateCI;
	pbSkybox.vertexInputStateCI = vertexInputInfo;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	pbSkybox.rasterisationStateCI = rasterizer;
	pbSkybox.multisampleStateCI = multisampling;
	pbSkybox.colourBlendStateCI = colourBlending;
	depthStencil.depthTestEnable = VK_FALSE;
	pbSkybox.depthStencilStateCI = depthStencil;
	pbSkybox.dynamicStateCI = dynamicState;
	pbSkybox.bufferedRenderPassIndex = (int)BRP::finall;
	pbSkybox.layeredBufferedRenderPassIndex = -1;
	
	// final
	EVK::GraphicsPipelineBlueprint pbFinal = {};
	pbFinal.pipelineBlueprint.descriptorSetsN = 1;
	EVK::DescriptorBlueprint finalDescriptorSetDescriptorBlueprints[1] = {
		{
			EVK::DescriptorType::combinedImageSampler,
			Pipeline_Final::textureBinding,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1,
			&finalColourImageIndex,
			&samplerMainIndex
		}
	};
	EVK::DescriptorSetBlueprint finalDescriptorSetBlueprints[1] = {{1, finalDescriptorSetDescriptorBlueprints}};
	pbFinal.pipelineBlueprint.descriptorSetBlueprints = finalDescriptorSetBlueprints;
	pbFinal.pipelineBlueprint.pushConstantRangesN = 0;
	pbFinal.stageCount = 2;
	VkPipelineShaderStageCreateInfo finalShaderStages[2];
	memcpy(finalShaderStages, shaderStages, 2*sizeof(VkPipelineShaderStageCreateInfo));
	finalShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertFinal.spv");
	finalShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragFinal.spv");
	pbFinal.shaderStageCIs = finalShaderStages;
	vertexInputInfo = Pipeline_Final::Attributes::stateCI;
	pbFinal.vertexInputStateCI = vertexInputInfo;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
	pbFinal.rasterisationStateCI = rasterizer;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	pbFinal.multisampleStateCI = multisampling;
	colourBlending.attachmentCount = 1;
	pbFinal.colourBlendStateCI = colourBlending;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	pbFinal.depthStencilStateCI = depthStencil;
	dynamicState.dynamicStateCount = 2;
	pbFinal.dynamicStateCI = dynamicState;
	pbFinal.bufferedRenderPassIndex = -1;
	pbFinal.layeredBufferedRenderPassIndex = -1;
	
	
	EVK::InterfaceBlueprint nvi = {devices};
	
	nvi.graphicsPipelinesN = GRAPHICS_PIPELINES_N;
	EVK::GraphicsPipelineBlueprint pbs[GRAPHICS_PIPELINES_N];
	pbs[(int)GraphicsPipeline::mainInstanced] = pbMainInstanced;
	pbs[(int)GraphicsPipeline::mainOnce] = pbMainOnce;
	pbs[(int)GraphicsPipeline::shadowInstanced] = pbShadowInstanced;
	pbs[(int)GraphicsPipeline::shadowOnce] = pbShadowOnce;
	pbs[(int)GraphicsPipeline::hud] = pbHud;
	pbs[(int)GraphicsPipeline::skybox] = pbSkybox;
	pbs[(int)GraphicsPipeline::finall] = pbFinal;
	nvi.graphicsPipelineBlueprints = pbs;
	
	EVK::ComputePipelineBlueprint cb;
	EVK::DescriptorBlueprint histogramDescriptorSetDescriptorBlueprints[3] = {
		{
			EVK::DescriptorType::UBO,
			Pipeline_Histogram::UBO::binding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			1,
			&histogramUboIndex
		},
		{
			EVK::DescriptorType::storageImage,
			Pipeline_Histogram::hdrImageBinding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			1,
			&finalColourImageIndex
		},
		{
			EVK::DescriptorType::SBO,
			Pipeline_Histogram::SBO::binding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			1,
			&histogramSboIndex
		}
	};
	EVK::DescriptorSetBlueprint histogramDescriptorSetBlueprint = {3, histogramDescriptorSetDescriptorBlueprints};
	cb.pipelineBlueprint.descriptorSetsN = 1;
	cb.pipelineBlueprint.descriptorSetBlueprints = &histogramDescriptorSetBlueprint;
	cb.pipelineBlueprint.pushConstantRangesN = 0;
	memcpy(&cb.shaderStageCI, shaderStages, sizeof(VkPipelineShaderStageCreateInfo));
	cb.shaderStageCI.module = devices.CreateShaderModule("../Resources/Shaders/histogram.spv");
	nvi.computePipelinesN = 1;
	nvi.computePipelineBlueprints = &cb;
	
	nvi.uniformBufferObjectsN = Globals::ubosN;
	nvi.uboBlueprints = (EVK::UniformBufferObjectBlueprint[Globals::ubosN]){
		{sizeof(Shared_Main::UBO_Global), 1},
		{sizeof(Shared_Main::PerObject), Globals::MainOnce::renderedN},
		{sizeof(Pipeline_Hud::UBO), 1},
		{sizeof(Shared_Shadow::UBO_Global), 1},
		{sizeof(Pipeline_Skybox::UBO_Global), 1},
		{sizeof(Pipeline_Histogram::UBO), 1}
	};
	
	nvi.storageBufferObjectsN = 1;
	nvi.sboBlueprints = (EVK::StorageBufferObjectBlueprint[1]){
		{sizeof(Pipeline_Histogram::SBO)}
	};
	
	nvi.textureSamplersN = SAMPLERS_N;
	VkSamplerCreateInfo sbs[SAMPLERS_N];
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = devices.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.unnormalizedCoordinates = VK_FALSE; // 'VK_TRUE' would mean texture coordinates are (0, texWidth), (0, texHeight)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f; // Optional
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = 20.0f; // max level of detail (miplevels)
	sbs[(int)Sampler::main] = samplerInfo;
	
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	sbs[(int)Sampler::cube] = samplerInfo;
	
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	VkFilter shadowmap_filter = (devices.GetFormatProperties(DEPTH_FORMAT).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	samplerInfo.magFilter = shadowmap_filter;
	samplerInfo.minFilter = shadowmap_filter;
	sbs[(int)Sampler::shadow] = samplerInfo;
	
	nvi.samplerBlueprints = sbs;
	
	// Creating the shadow mapping render pass
	VkAttachmentDescription lbAttachmentDescription{};
	lbAttachmentDescription.format = DEPTH_FORMAT;
	lbAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	lbAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
	lbAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
	lbAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	lbAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	lbAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
	lbAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end
	
	VkAttachmentReference lbDepthReference = {};
	lbDepthReference.attachment = 0;
	lbDepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass
	
	VkSubpassDescription lbSubpass = {};
	lbSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	lbSubpass.colorAttachmentCount = 0;													// No color attachments
	lbSubpass.pDepthStencilAttachment = &lbDepthReference;									// Reference to our depth attachment
	
	// Use subpass dependencies for layout transitions
	VkSubpassDependency lbDependencies[2];
	lbDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	lbDependencies[0].dstSubpass = 0;
	lbDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	lbDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	lbDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	lbDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	lbDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	lbDependencies[1].srcSubpass = 0;
	lbDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	lbDependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	lbDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	lbDependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	lbDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	lbDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	VkRenderPassCreateInfo lbBenderPassCreateInfo = {};
	lbBenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	lbBenderPassCreateInfo.attachmentCount = 1;
	lbBenderPassCreateInfo.pAttachments = &lbAttachmentDescription;
	lbBenderPassCreateInfo.subpassCount = 1;
	lbBenderPassCreateInfo.pSubpasses = &lbSubpass;
	lbBenderPassCreateInfo.dependencyCount = 2;
	lbBenderPassCreateInfo.pDependencies = lbDependencies;
	EVK::LayeredBufferedRenderPassBlueprint lbrpb = {
		lbBenderPassCreateInfo,
		shadowMapImageIndex,
		SHADOWMAP_DIM,
		SHADOWMAP_DIM,
		SHADOW_MAP_CASCADE_COUNT,
		DEPTH_FORMAT,
		VK_IMAGE_ASPECT_DEPTH_BIT
	};
	nvi.layeredBufferedRenderPassesN = LAYERED_BUFFERED_RENDER_PASSES_N;
	nvi.layeredBufferedRenderPassBlueprints = &lbrpb;
	
	// Creating the final render pass
	static VkAttachmentDescription colourAttachment{};
	colourAttachment.format = FINAL_FORMAT;
#ifdef MSAA
	colourAttachment.samples = devices.GetMSAASamples();
#else
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	static VkAttachmentReference colourAttachmentRef{};
	colourAttachmentRef.attachment = 0;
	colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	static VkAttachmentDescription depthAttachment{};
	depthAttachment.format = devices.FindDepthFormat();
#ifdef MSAA
	depthAttachment.samples = devices.GetMSAASamples();
#else
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
#endif
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	static VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	static VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
	// Use subpass dependencies for layout transitions
	static VkSubpassDependency bDependencies[2];
	bDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	bDependencies[0].dstSubpass = 0;
	bDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	bDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	bDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	bDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	bDependencies[1].srcSubpass = 0;
	bDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	bDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	bDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	bDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	bDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	static VkRenderPassCreateInfo bRenderPassCreateInfo = {};
	bRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	bRenderPassCreateInfo.attachmentCount = 2;
	static VkAttachmentDescription attachments[2] = {colourAttachment, depthAttachment};
	bRenderPassCreateInfo.pAttachments = attachments;
	bRenderPassCreateInfo.subpassCount = 1;
	bRenderPassCreateInfo.pSubpasses = &subpass;
	bRenderPassCreateInfo.dependencyCount = 2;
	bRenderPassCreateInfo.pDependencies = bDependencies;
	static int brpImages[2] = {finalColourImageIndex, finalDepthImageIndex};
	EVK::BufferedRenderPassBlueprint brpb = {
		bRenderPassCreateInfo,
		2,
		brpImages,
		0, // resises with window
		0 //
	};
	nvi.bufferedRenderPassesN = BUFFERED_RENDER_PASSES_N;
	nvi.bufferedRenderPassBlueprints = &brpb;
	
	nvi.vertexBuffersN = Globals::vertexBuffersN;
	nvi.indexBuffersN = Globals::indexBuffersN;
	nvi.textureImagesN = Globals::texturesN;
	nvi.imageBlueprintPtrs = imageBlueprintPtrs;
	
	return EVK::Interface(nvi);
}
