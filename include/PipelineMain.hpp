#pragma once

#include "Header.hpp"

namespace PipelineMain {

struct Vertex {
	vec<3, float32_t> position;
	vec<3, float32_t> normal;
	vec<2, float32_t> texCoord;
};

struct UBO_Global {
	mat<4, 4, float32_t> lightMat[SHADOW_MAP_CASCADE_COUNT];
	mat<4, 4, float32_t> viewInv;
	mat<4, 4, float32_t> proj;
	vec<4, float32_t> lightColour;
	float32_t cascadeSplits[SHADOW_MAP_CASCADE_COUNT]; // currently implemented as a vec4
	vec<4, float32_t> lightDir; // only using first three components
	vec<4, float32_t> cameraPosition; // only using first three components
};

namespace FragmentShader {

static constexpr char fragmentFilename[] = "../Resources/Shaders/fragMain.spv";

using PCS = EVK::PushConstants<16, Shared_Main::PushConstants_Frag>;
static_assert(EVK::pushConstants_c<PCS>);

using type = EVK::Shader<VK_SHADER_STAGE_FRAGMENT_BIT, fragmentFilename, PCS,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::TextureSamplersUniform<0, 1, 1>,
EVK::TextureImagesUniform<0, 2, PNGS_N>,
EVK::CombinedImageSamplersUniform<0, 3, 1>
>;
static_assert(EVK::shader_c<type>);

} // namespace FragmentShader

namespace Instanced {

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertMainInstanced.spv";

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, AttributesInstanced,
EVK::UBOUniform<0, 0, UBO_Global>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

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
	
	
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
#ifdef MSAA
	multisampling.rasterizationSamples = interface->devices->GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	
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

static constexpr char vertexFilename[] = "../Resources/Shaders/vertMainOnce.spv";

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, AttributesOnce,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::UBOUniform<1, 0, PerObject, true>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

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
	
	
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
#ifdef MSAA
	multisampling.rasterizationSamples = interface->devices->GetMSAASamples();
#else
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
	colourBlending.attachmentCount = 1;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	dynamicState.dynamicStateCount = 2;
	
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

} // namespace PipelineMain

//struct Pipeline_MainInstanced {
//	static const int descriptorSetsN = 1;
//
//	struct Attributes {
//		struct Vertex {
//			float32_t position[3];
//			float32_t normal[3];
//			float32_t texCoord[2];
//		};
//
//		static constexpr VkPipelineVertexInputStateCreateInfo stateCI = {
//			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//			nullptr,
//			NULL,
//			2,
//			(VkVertexInputBindingDescription[2]){
//				{
//					0, // binding
//					32,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_VERTEX // input rate
//				},
//				{
//					1, // binding
//					128,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_INSTANCE // input rate
//				}
//			},
//			11,
//			(VkVertexInputAttributeDescription[11]){
//				{
//					0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0//offsetof(Vertex, position)
//				},
//				{
//					1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12//offsetof(Vertex, normal)
//				},
//				{
//					2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
//				},
//				{
//					3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0//offsetof(Vertex, position)
//				},
//				{
//					4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 16//offsetof(Vertex, position)
//				},
//				{
//					5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 32//offsetof(Vertex, position)
//				},
//				{
//					6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 48//offsetof(Vertex, position)
//				},
//				{
//					7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 64//offsetof(Vertex, normal)
//				},
//				{
//					8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 80//offsetof(Vertex, normal)
//				},
//				{
//					9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 96//offsetof(Vertex, normal)
//				},
//				{
//					10, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 112//offsetof(Vertex, normal)
//				}
//			}
//		};
//	};
//};

//struct Pipeline_MainOnce {
//	// shared 'PerObject' is a UBO in this pipeline
//	static const int perObjectUboBinding = 0;
//	static const int dynamicOffsetsN = 1;
//	static const int descriptorSetsN = 2;
//
//	struct Attributes {
//		struct Vertex {
//			vec<3, float32_t> position;
//			vec<3, float32_t> normal;
//			vec<2, float32_t> texCoord;
//		};
//
//		static constexpr VkPipelineVertexInputStateCreateInfo stateCI = {
//			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//			nullptr,
//			NULL,
//			1,
//			(VkVertexInputBindingDescription[1]){
//				{
//					0, // binding
//					32,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_VERTEX // input rate
//				}
//			},
//			3,
//			(VkVertexInputAttributeDescription[3]){
//				{
//					0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0//offsetof(Vertex, position)
//				},
//				{
//					1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12//offsetof(Vertex, normal)
//				},
//				{
//					2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
//				}
//			}
//		};
//	};
//};

//struct Shared_Main {
//	struct PushConstants_Vert {
//		int32_t placeHolder[4];
//	};
//	struct PushConstants_Frag {
//		vec<4, float32_t> colourMult;
//		vec<4, float32_t> specular;
//		float32_t shininess;
//		float32_t specularFactor;
//		int32_t textureID;
//	};
//	enum class PushConstantRange {vert, frag};
//
//	struct UBO_Global {
//		mat<4, 4, float32_t> lightMat[SHADOW_MAP_CASCADE_COUNT];
//		mat<4, 4, float32_t> viewInv;
//		mat<4, 4, float32_t> proj;
//		vec<4, float32_t> lightColour;
//		float32_t cascadeSplits[SHADOW_MAP_CASCADE_COUNT]; // currently implemented as a vec4
//		vec<4, float32_t> lightDir; // only using first three components
//		vec<4, float32_t> cameraPosition; // only using first three components
//
//		static const int binding = 0;
//	};
//
//	struct PerObject {
//		mat<4, 4, float32_t> model;
//		mat<4, 4, float32_t> modelInvT;
//	};
//
//	static const int ubosN = 2;
//
//	static const int textureSamplerBinding = 1;
//	static const int textureImagesBinding = 2;
//	static const int shadowMapBinding = 3;
//};
