#include "Pipelines.hpp"

extern std::vector<int> textureImageIndexArray;

std::shared_ptr<EVK::Interface> NewBuildPipelines(const EVK::Devices &devices, const std::vector<std::shared_ptr<EVK::IImageBlueprint>> &imageBlueprintPtrs, int shadowMapImageIndex, int skyboxImageIndex, int finalColourImageIndex, int finalDepthImageIndex){
	
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
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};
	
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{// vertex shader
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.pName = "main",
			//.module = ...,
			// this is for constants to use in the shader:
			.pSpecializationInfo = nullptr
		},
		{// fragment shader
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pName = "main",
			//.module = ...,
			// this is for constants to use in the shader:
			.pSpecializationInfo = nullptr
		}
	};
	
	
	std::vector<VkPushConstantRange> mainPushConstantRanges = {
		(VkPushConstantRange){
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(Shared_Main::PushConstants_Vert)
		},
		(VkPushConstantRange){
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = sizeof(Shared_Main::PushConstants_Vert),
			.size = sizeof(Shared_Main::PushConstants_Frag)
		}
	};
	VkPushConstantRange shadowPushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(Shared_Shadow::PushConstants_Vert)
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
	
	EVK::DescriptorSetBlueprint mainDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::UBO,
			.binding = Shared_Main::UBO_Global::binding,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {uboMainGlobalIndex}
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::textureImage,
			.binding = Shared_Main::textureImagesBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = textureImageIndexArray
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::textureSampler,
			.binding = Shared_Main::textureSamplerBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {samplerMainIndex}
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::combinedImageSampler,
			.binding = Shared_Main::shadowMapBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {shadowMapImageIndex},
			.indicesExtra2 = {samplerShadowIndex}
		}
	};
	EVK::DescriptorSetBlueprint shadowDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::UBO,
			.binding = Shared_Shadow::UBO_Global::binding,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.indicesExtra = {uboShadowIndex}
		}
	};
	
	std::vector<EVK::DescriptorBlueprint> onceDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::UBO,
			.binding = Pipeline_MainOnce::perObjectUboBinding,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.indicesExtra = {uboMainPerObjectIndex}
		}
	};
	
	// main instanced
	std::vector<VkPipelineShaderStageCreateInfo> mainInstancedShaderStages = shaderStages;
	mainInstancedShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertMainInstanced.spv");
	mainInstancedShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragMain.spv");
	vertexInputInfo = Pipeline_MainInstanced::Attributes::stateCI;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	EVK::GraphicsPipelineBlueprint pbMainInstanced = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {mainDescriptorSetBlueprint},
			.pushConstantRanges = mainPushConstantRanges
		},
			.shaderStageCIs = mainInstancedShaderStages,
			.vertexInputStateCI = vertexInputInfo,
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {(int)BRP::finall},
			.layeredBufferedRenderPassIndex = {}
	};
	
	// shadow instanced
	VkPipelineShaderStageCreateInfo shadowInstancedShaderStage = shaderStages[0];
	shadowInstancedShaderStage.module = devices.CreateShaderModule("../Resources/Shaders/vertShadowInstanced.spv");
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	colourBlending.attachmentCount = 0;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dynamicState.dynamicStateCount = 3;
	EVK::GraphicsPipelineBlueprint pbShadowInstanced = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {shadowDescriptorSetBlueprint},
			.pushConstantRanges = {shadowPushConstantRange}
		},
			.shaderStageCIs = {shadowInstancedShaderStage},
			.vertexInputStateCI = vertexInputInfo, // same as mainInstanced
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {},
			.layeredBufferedRenderPassIndex = {(int)LBRP::shadow}
	};
	
	
	// main once
	std::vector<VkPipelineShaderStageCreateInfo> mainOnceShaderStages = shaderStages;
	mainOnceShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertMainOnce.spv");
	mainOnceShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragMain.spv");
	vertexInputInfo = Pipeline_MainOnce::Attributes::stateCI;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	EVK::GraphicsPipelineBlueprint pbMainOnce = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {
				mainDescriptorSetBlueprint,
				onceDescriptorSetBlueprint
			},
				.pushConstantRanges = mainPushConstantRanges
		},
			.shaderStageCIs = mainOnceShaderStages,
			.vertexInputStateCI = vertexInputInfo,
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {(int)BRP::finall},
			.layeredBufferedRenderPassIndex = {}
	};
	
	// shadow once
	VkPipelineShaderStageCreateInfo shadowOnceShaderStage = shaderStages[0];
	shadowOnceShaderStage.module = devices.CreateShaderModule("../Resources/Shaders/vertShadowOnce.spv");
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	colourBlending.attachmentCount = 0;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dynamicState.dynamicStateCount = 3;
	EVK::GraphicsPipelineBlueprint pbShadowOnce = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {
				shadowDescriptorSetBlueprint,
				onceDescriptorSetBlueprint
			},
				.pushConstantRanges = {shadowPushConstantRange}
		},
			.shaderStageCIs = {shadowOnceShaderStage},
			.vertexInputStateCI = vertexInputInfo, // same as mainOnce
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {},
			.layeredBufferedRenderPassIndex = {(int)LBRP::shadow}
	};
	
	
	// hud
	int hudImageIndex = 4;
	EVK::DescriptorSetBlueprint hudDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::UBO,
			.binding = Pipeline_Hud::UBO::binding,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.indicesExtra = {uboHudIndex}
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::textureSampler,
			.binding = Pipeline_Hud::textureSamplerBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {samplerMainIndex}
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::textureImage,
			.binding = Pipeline_Hud::textureImagesBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {hudImageIndex}
		}
	};
	std::vector<VkPipelineShaderStageCreateInfo> hudShaderStages = shaderStages;
	hudShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertHud.spv");
	hudShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragHud.spv");
	vertexInputInfo = Pipeline_Hud::Attributes::stateCI;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	EVK::GraphicsPipelineBlueprint pbHud = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {hudDescriptorSetBlueprint},
			.pushConstantRanges = {}
		},
			.shaderStageCIs = hudShaderStages,
			.vertexInputStateCI = vertexInputInfo,
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {(int)BRP::finall},
			.layeredBufferedRenderPassIndex = {}
	};
	
	
	// skybox
	EVK::DescriptorSetBlueprint skyboxDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::UBO,
			.binding = Pipeline_Skybox::UBO_Global::binding,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {uboSkyboxIndex}
		},
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::combinedImageSampler,
			.binding = Pipeline_Skybox::cubemapBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {skyboxImageIndex},
			.indicesExtra2 = {skyboxSamplerIndex}
		}
	};
	std::vector<VkPipelineShaderStageCreateInfo> skyboxShaderStages = shaderStages;
	skyboxShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertSkybox.spv");
	skyboxShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragSkybox.spv");
	vertexInputInfo = Pipeline_Skybox::Attributes::stateCI;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	depthStencil.depthTestEnable = VK_FALSE;
	EVK::GraphicsPipelineBlueprint pbSkybox = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {skyboxDescriptorSetBlueprint},
			.pushConstantRanges = {}
		},
			.shaderStageCIs = skyboxShaderStages,
			.vertexInputStateCI = vertexInputInfo,
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {(int)BRP::finall},
			.layeredBufferedRenderPassIndex = {}
	};
	
	// final
	EVK::DescriptorSetBlueprint finalDescriptorSetBlueprint = {
		(EVK::DescriptorBlueprint){
			.type = EVK::DescriptorType::combinedImageSampler,
			.binding = Pipeline_Final::textureBinding,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.indicesExtra = {finalColourImageIndex},
			.indicesExtra2 = {samplerMainIndex}
		}
	};
	std::vector<VkPipelineShaderStageCreateInfo> finalShaderStages = shaderStages;
	finalShaderStages[0].module = devices.CreateShaderModule("../Resources/Shaders/vertFinal.spv");
	finalShaderStages[1].module = devices.CreateShaderModule("../Resources/Shaders/fragFinal.spv");
	vertexInputInfo = Pipeline_Final::Attributes::stateCI;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
#ifdef MSAA
	multisampling.rasterizationSamples = devices.GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	EVK::GraphicsPipelineBlueprint pbFinal = {
		.pipelineBlueprint = {
			.descriptorSetBlueprints = {finalDescriptorSetBlueprint},
			.pushConstantRanges = {}
		},
			.shaderStageCIs = finalShaderStages,
			.vertexInputStateCI = vertexInputInfo,
			.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.rasterisationStateCI = rasterizer,
			.multisampleStateCI = multisampling,
			.colourBlendStateCI = colourBlending,
			.depthStencilStateCI = depthStencil,
			.dynamicStateCI = dynamicState,
			.bufferedRenderPassIndex = {},
			.layeredBufferedRenderPassIndex = {}
	};
	
	
	EVK::InterfaceBlueprint nvi = {
		.devices = devices
	};
	
	std::vector<EVK::GraphicsPipelineBlueprint> pbs(GRAPHICS_PIPELINES_N);
	pbs[(int)GraphicsPipeline::mainInstanced] = pbMainInstanced;
	pbs[(int)GraphicsPipeline::mainOnce] = pbMainOnce;
	pbs[(int)GraphicsPipeline::shadowInstanced] = pbShadowInstanced;
	pbs[(int)GraphicsPipeline::shadowOnce] = pbShadowOnce;
	pbs[(int)GraphicsPipeline::hud] = pbHud;
	pbs[(int)GraphicsPipeline::skybox] = pbSkybox;
	pbs[(int)GraphicsPipeline::finall] = pbFinal;
	nvi.graphicsPipelineBlueprints = pbs;
	
	EVK::ComputePipelineBlueprint cb;
	EVK::DescriptorSetBlueprint histogramDescriptorSetBlueprint = {
		{
			EVK::DescriptorType::UBO,
			Pipeline_Histogram::UBO::binding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			{histogramUboIndex}
		},
		{
			EVK::DescriptorType::storageImage,
			Pipeline_Histogram::hdrImageBinding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			{finalColourImageIndex}
		},
		{
			EVK::DescriptorType::SBO,
			Pipeline_Histogram::SBO::binding,
			VK_SHADER_STAGE_COMPUTE_BIT,
			{histogramSboIndex}
		}
	};
	cb.pipelineBlueprint.descriptorSetBlueprints = {histogramDescriptorSetBlueprint};
	cb.pipelineBlueprint.pushConstantRanges = {};
	cb.shaderStageCI = shaderStages[0];
	cb.shaderStageCI.module = devices.CreateShaderModule("../Resources/Shaders/histogram.spv");
	nvi.computePipelineBlueprints = {cb};
	
	nvi.uboBlueprints = {
		{sizeof(Shared_Main::UBO_Global), {}},
		{sizeof(Shared_Main::PerObject), {Globals::MainOnce::renderedN}},
		{sizeof(Pipeline_Hud::UBO), {}},
		{sizeof(Shared_Shadow::UBO_Global), {}},
		{sizeof(Pipeline_Skybox::UBO_Global), {}},
		{sizeof(Pipeline_Histogram::UBO), {}}
	};
	
	// !!! no idea if the bit masks are correct for this
	nvi.sboBlueprints = {(EVK::StorageBufferObjectBlueprint){sizeof(Pipeline_Histogram::SBO), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};
	
	nvi.samplerBlueprints = std::vector<VkSamplerCreateInfo>(SAMPLERS_N);
	VkSamplerCreateInfo samplerInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = devices.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy,
		.unnormalizedCoordinates = VK_FALSE, // 'VK_TRUE' would mean texture coordinates are (0, texWidth), (0, texHeight)
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f, // Optional
		.minLod = 0.0f, // Optional
		.maxLod = 20.0f // max level of detail (miplevels)
	};
	nvi.samplerBlueprints[(int)Sampler::main] = samplerInfo;
	
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	nvi.samplerBlueprints[(int)Sampler::cube] = samplerInfo;
	
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	const VkFilter shadowmap_filter = (devices.GetFormatProperties(DEPTH_FORMAT).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	samplerInfo.magFilter = shadowmap_filter;
	samplerInfo.minFilter = shadowmap_filter;
	nvi.samplerBlueprints[(int)Sampler::shadow] = samplerInfo;
	
	// Creating the shadow mapping render pass
	VkAttachmentDescription lbAttachmentDescription{
		.format = DEPTH_FORMAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,							// Clear depth at beginning of the render pass
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,						// We will read from depth, so it's important to store the depth attachment results
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,					// We don't care about initial layout of the attachment
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL// Attachment will be transitioned to shader read at render pass end
	};
	
	VkAttachmentReference lbDepthReference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL			// Attachment will be used as depth/stencil during render pass
	};
		
	VkSubpassDescription lbSubpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 0,													// No color attachments
		.pDepthStencilAttachment = &lbDepthReference									// Reference to our depth attachment
	};
		
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
	
	VkRenderPassCreateInfo lbBenderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &lbAttachmentDescription,
		.subpassCount = 1,
		.pSubpasses = &lbSubpass,
		.dependencyCount = 2,
		.pDependencies = lbDependencies
	};
	EVK::LayeredBufferedRenderPassBlueprint lbrpb = {
		.renderPassCI = lbBenderPassCreateInfo,
		.targetTextureImageIndex = shadowMapImageIndex,
		.width = SHADOWMAP_DIM,
		.height = SHADOWMAP_DIM,
		.layersN = SHADOW_MAP_CASCADE_COUNT,
		.imageFormat = DEPTH_FORMAT,
		.imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT
	};
	nvi.layeredBufferedRenderPassBlueprints = {lbrpb};
	
	// Creating the final render pass
	static VkAttachmentDescription colourAttachment{
		.format = FINAL_FORMAT,
#ifdef MSAA
		.samples = devices.GetMSAASamples(),
#else
		.samples = VK_SAMPLE_COUNT_1_BIT,
#endif
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	static VkAttachmentReference colourAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	
	static VkAttachmentDescription depthAttachment{
		.format = devices.FindDepthFormat(),
#ifdef MSAA
		.samples = devices.GetMSAASamples(),
#else
		.samples = VK_SAMPLE_COUNT_1_BIT,
#endif
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	static VkAttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	static VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colourAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};
	
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
	
	static VkAttachmentDescription attachments[2] = {colourAttachment, depthAttachment};
	static VkRenderPassCreateInfo bRenderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 2,
		.pDependencies = bDependencies
	};
	EVK::BufferedRenderPassBlueprint brpb = {
		.renderPassCI = bRenderPassCreateInfo,
		.targetTextureImageIndices = {finalColourImageIndex, finalDepthImageIndex},
		.width = 0, // resises with window
	};
	nvi.bufferedRenderPassBlueprints = {brpb};
	
	nvi.vertexBuffersN = Globals::vertexBuffersN;
	nvi.indexBuffersN = Globals::indexBuffersN;
	nvi.imageBlueprintPtrs = imageBlueprintPtrs;
	
	return std::make_shared<EVK::Interface>(nvi);
}
