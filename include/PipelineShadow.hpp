#pragma once

#include "Header.hpp"

namespace PipelineShadow {

using PCS = EVK::PushConstants<0, Shared_Main::PushConstants_Vert>;
static_assert(EVK::pushConstants_c<PCS>);

struct UBO_Global {
	mat<4, 4, float32_t> viewInvProj[SHADOW_MAP_CASCADE_COUNT];
};

namespace Instanced {

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertShadowInstanced.spv";

using type = EVK::VertexShader<vertexFilename, PCS, AttributesInstanced,
EVK::UBOUniform<0, 0, UBO_Global>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

using type = EVK::DepthPipeline<VertexShader::type>;

inline std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle){
	
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	
	// ----- Multisampling behaviour -----
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	
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
	colourBlending.pAttachments = &colourBlendAttachment;
	
	// ----- Depth stencil behaviour -----
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;
	
	// ----- Dynamic state -----
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkDynamicState dynamicStates[3] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
	};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStates;
	
	
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	colourBlending.attachmentCount = 0;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dynamicState.dynamicStateCount = 3;
	
	const EVK::RenderPipelineBlueprint blueprint = {
		.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.pRasterisationStateCI = &rasterizer,
		.pMultisampleStateCI = &multisampling,
		.pDepthStencilStateCI = &depthStencil,
		.pColourBlendStateCI = &colourBlending,
		.pDynamicStateCI = &dynamicState,
		.renderPassHandle = renderPassHandle
	};
	
	return std::make_shared<type>(devices, &blueprint);
}

} // namespace Instanced

namespace Once {

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertShadowOnce.spv";

using type = EVK::VertexShader<vertexFilename, PCS, AttributesOnce,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::UBOUniform<1, 0, PerObject, true>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

using type = EVK::DepthPipeline<VertexShader::type>;

inline std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle){
	
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	
	// ----- Multisampling behaviour -----
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	
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
	colourBlending.pAttachments = &colourBlendAttachment;
	
	// ----- Depth stencil behaviour -----
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;
	
	// ----- Dynamic state -----
	VkPipelineDynamicStateCreateInfo dynamicState{};
	VkDynamicState dynamicStates[3] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
	};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStates;
	
	
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_TRUE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	colourBlending.attachmentCount = 0;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dynamicState.dynamicStateCount = 3;
	
	const EVK::RenderPipelineBlueprint blueprint = {
		.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.pRasterisationStateCI = &rasterizer,
		.pMultisampleStateCI = &multisampling,
		.pDepthStencilStateCI = &depthStencil,
		.pColourBlendStateCI = &colourBlending,
		.pDynamicStateCI = &dynamicState,
		.renderPassHandle = renderPassHandle
	};
	
	return std::make_shared<type>(devices, &blueprint);
}

} // namespace Once

} // namespace PipelineShadow




//struct Pipeline_ShadowInstanced {
//	static const int descriptorSetsN = 1;
//};
//struct Pipeline_ShadowOnce {
//	static const int descriptorSetsN = 2;
//};
//struct Shared_Shadow {
//	struct UBO_Global {
//		mat<4, 4, float32_t> viewInvProj[SHADOW_MAP_CASCADE_COUNT];
//
//		static const int binding = 0;
//	};
//	static const int ubosN = 1;
//	struct PushConstants_Vert {
//		int32_t cascadeLayer;
//	};
//};

