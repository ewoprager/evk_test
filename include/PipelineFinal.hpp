#pragma once

#include "Header.hpp"

namespace PipelineFinal {

struct Vertex {
	vec<2, float32_t> position;
	vec<2, float32_t> texCoord;
};

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertFinal.spv";

using Attributes = EVK::Attributes<EVK::BindingDescriptionPack<
VkVertexInputBindingDescription{
	0, // binding
	16, // stride
	VK_VERTEX_INPUT_RATE_VERTEX // input rate
}
>, EVK::AttributeDescriptionPack<
VkVertexInputAttributeDescription{
	0, 0, VK_FORMAT_R32G32_SFLOAT, 0//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	1, 0, VK_FORMAT_R32G32_SFLOAT, 8//offsetof(Vertex, texCoord)
}
>>;

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, Attributes>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

namespace FragmentShader {

static constexpr char fragmentFilename[] = "../Resources/Shaders/fragFinal.spv";

using type = EVK::Shader<VK_SHADER_STAGE_FRAGMENT_BIT, fragmentFilename, EVK::NoPushConstants,
EVK::CombinedImageSamplersUniform<0, 0, 1>
>;
static_assert(EVK::shader_c<type>);

} // namespace FragmentShader

using type = EVK::RenderPipeline<VertexShader::type, FragmentShader::type>;

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
	
	const EVK::RenderPipelineBlueprint blueprint = {
		.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.pRasterisationStateCI = &rasterizer,
		.pMultisampleStateCI = &multisampling,
		.pDepthStencilStateCI = &depthStencil,
		.pColourBlendStateCI = &colourBlending,
		.pDynamicStateCI = &dynamicState,
		.renderPassHandle = renderPassHandle
	};
	
	
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
#ifdef MSAA
	multisampling.rasterizationSamples = interface->devices->GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	depthStencil.depthTestEnable = VK_FALSE;
	
	return std::make_shared<type>(devices, &blueprint);
}

} // namespace PipelineFinal



//struct Pipeline_Final {
//	static const int textureBinding = 0;
//
//	struct Attributes {
//		struct Vertex {
//			vec<2, float32_t> position;
//			vec<2, float32_t> texCoord;
//		};
//
//
//		static constexpr VkPipelineVertexInputStateCreateInfo stateCI = {
//			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//			nullptr,
//			NULL,
//			1,
//			(VkVertexInputBindingDescription[1]){
//				{
//					0, // binding
//					16,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_VERTEX // input rate
//				}
//			},
//			2,
//			(VkVertexInputAttributeDescription[2]){
//				{
//					0, 0, VK_FORMAT_R32G32_SFLOAT, 0//offsetof(Vertex, position)
//				},
//				{
//					1, 0, VK_FORMAT_R32G32_SFLOAT, 8//offsetof(Vertex, texCoord)
//				}
//			}
//		};
//	};
//};
