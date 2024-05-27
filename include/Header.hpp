#ifndef Header_hpp
#define Header_hpp

#include <map>
#include <sys/time.h>

#include <mattresses.h>
#include <ESDL/ESDL_General.h>
#include <ESDL/ESDL_EventHandler.h>
#include <SDL2/SDL_vulkan.h>
#include <evk/ShaderProgram.hpp>

#include <ReadProcessedObj.hpp>


#define GRAPHICS_PIPELINES_N 7
enum class GraphicsPipeline {mainInstanced, mainOnce, hud, shadowInstanced, shadowOnce, skybox, finall};

#define SAMPLERS_N 3
enum class Sampler {main, cube, shadow, _COUNT_};

enum class UBO {mainGlobal, mainPerObject, hud, shadow, skybox, histogram};
enum class SBO {histogram};

#define BUFFERED_RENDER_PASSES_N 1
enum class BRP {finall};

#define LAYERED_BUFFERED_RENDER_PASSES_N 1
enum class LBRP {shadow};

enum class VertexBufferBinding {vertex, instance}; // binding locations for vertex buffers: per-vertex buffers are at binding 0 and per-devices.instance buffers are at binding 1

#define MAX_INSTANCES 10000

#define FINAL_FORMAT VK_FORMAT_R32G32B32A32_SFLOAT

// texture images:
#define OTHER_IMAGES_N 4
enum class OtherImage {skybox, shadow_cascades, colour, depth};

#define PNGS_N 4 // debug, chair, chainsaw, concrete

#define SHADOW_MAP_CASCADE_COUNT 4 // current implementation requires this to be 4, as cascade split values are passed to shader as a vec4.
#define SHADOWMAP_DIM 2048
// 16 bits of depth is enough for such a small scene
#define DEPTH_FORMAT VK_FORMAT_D16_UNORM


// Tools
unsigned long UTime();
uint32_t GetTextureIdFromMtl(const char *usemtl);


// -----
// Pipelines
// -----

using AttributesInstanced = EVK::Attributes<EVK::BindingDescriptionPack<
VkVertexInputBindingDescription{
	0, // binding
	32,//sizeof(Vertex), // stride
	VK_VERTEX_INPUT_RATE_VERTEX // input rate
},
VkVertexInputBindingDescription{
	1, // binding
	128,//sizeof(Vertex), // stride
	VK_VERTEX_INPUT_RATE_INSTANCE // input rate
}
>, EVK::AttributeDescriptionPack<
VkVertexInputAttributeDescription{
	0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12//offsetof(Vertex, normal)
},
VkVertexInputAttributeDescription{
	2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
},
VkVertexInputAttributeDescription{
	3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 16//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 32//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 48//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 64//offsetof(Vertex, normal)
},
VkVertexInputAttributeDescription{
	8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 80//offsetof(Vertex, normal)
},
VkVertexInputAttributeDescription{
	9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 96//offsetof(Vertex, normal)
},
VkVertexInputAttributeDescription{
	10, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 112//offsetof(Vertex, normal)
}
>>;

using AttributesOnce = EVK::Attributes<EVK::BindingDescriptionPack<
VkVertexInputBindingDescription{
	0, // binding
	32,//sizeof(Vertex), // stride
	VK_VERTEX_INPUT_RATE_VERTEX // input rate
}
>, EVK::AttributeDescriptionPack<
VkVertexInputAttributeDescription{
	0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12//offsetof(Vertex, normal)
},
VkVertexInputAttributeDescription{
	2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
}
>>;

struct PerObject {
	mat<4, 4, float32_t> model;
	mat<4, 4, float32_t> modelInvT;
};

namespace Shared_Main {

struct PushConstants_Vert {
	int32_t cascadeLayer;
};

struct PushConstants_Frag {
	vec<4, float32_t> colourMult;
	vec<4, float32_t> specular;
	float32_t shininess;
	float32_t specularFactor;
	int32_t textureID;
};

} // namespace Shared_Main

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

using PCS = EVK::PushConstants<16, PushConstants_Frag>;
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

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

} // namespace Instanced

namespace Once {

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertMainOnce.spv";

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, AttributesOnce,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::UBOUniform<1, 0, PerObject>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

using type = EVK::RenderPipeline<VertexShader::type, FragmentShader::type>;

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

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



namespace PipelineHud {

struct Vertex {
	vec<2, float32_t> position;
	vec<4, float32_t> colour;
	vec<2, float32_t> texCoord;
};

struct UBO {
	float32_t extentWidth;
	float32_t extentHeight;
	float32_t gap;
};

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertHud.spv";

using Attributes = EVK::Attributes<EVK::BindingDescriptionPack<
VkVertexInputBindingDescription{
	0, // binding
	32, // stride
	VK_VERTEX_INPUT_RATE_VERTEX // input rate
}
>, EVK::AttributeDescriptionPack<
VkVertexInputAttributeDescription{
	0, 0, VK_FORMAT_R32G32_SFLOAT, 0//offsetof(Vertex, position)
},
VkVertexInputAttributeDescription{
	1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 8//offsetof(Vertex, texCoord)
},
VkVertexInputAttributeDescription{
	2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
}
>>;

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, Attributes,
EVK::UBOUniform<0, 1, UBO>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

namespace FragmentShader {

static constexpr char fragmentFilename[] = "../Resources/Shaders/fragHud.spv";

using type = EVK::Shader<VK_SHADER_STAGE_FRAGMENT_BIT, fragmentFilename, EVK::NoPushConstants,
EVK::TextureSamplersUniform<0, 0, 1>,
EVK::TextureImagesUniform<0, 2, 1>
>;
static_assert(EVK::shader_c<type>);

} // namespace FragmentShader

using type = EVK::RenderPipeline<VertexShader::type, FragmentShader::type>;

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

} // namespace PipelineHud




//struct Pipeline_Hud {
//	struct UBO {
//		float32_t extentWidth;
//		float32_t extentHeight;
//		float32_t gap;
//		
//		static const int binding = 1;
//	};
//	static const int ubosN = 1;
//	
//	static const int descriptorSetsN = 1;
//	
//	static const int textureSamplerBinding = 0;
//	static const int textureImagesBinding = 2;
//	
//	struct Attributes {
//		struct Vertex {
//			vec<2, float32_t> position;
//			vec<4, float32_t> colour;
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
//					32,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_VERTEX // input rate
//				}
//			},
//			3,
//			(VkVertexInputAttributeDescription[3]){
//				{
//					0, 0, VK_FORMAT_R32G32_SFLOAT, 0//offsetof(Vertex, position)
//				},
//				{
//					1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 8//offsetof(Vertex, texCoord)
//				},
//				{
//					2, 0, VK_FORMAT_R32G32_SFLOAT, 24//offsetof(Vertex, texCoord)
//				}
//			}
//		};
//	};
//};



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

using type = EVK::RenderPipeline<VertexShader::type>;

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

} // namespace Instanced

namespace Once {

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertShadowOnce.spv";

using type = EVK::VertexShader<vertexFilename, PCS, AttributesOnce,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::UBOUniform<1, 0, PerObject>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

using type = EVK::RenderPipeline<VertexShader::type>;

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

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



namespace PipelineSkybox {

struct Vertex {
	vec<3, float32_t> position;
};

struct UBO_Global {
	mat<4, 4, float32_t> viewInv;
	mat<4, 4, float32_t> proj;
	vec<4, float32_t> cameraPosition; // only using first three components
};

namespace VertexShader {

static constexpr char vertexFilename[] = "../Resources/Shaders/vertSkybox.spv";

using Attributes = EVK::Attributes<EVK::BindingDescriptionPack<
VkVertexInputBindingDescription{
	0, // binding
	12, // stride
	VK_VERTEX_INPUT_RATE_VERTEX // input rate
}
>, EVK::AttributeDescriptionPack<
VkVertexInputAttributeDescription{
	0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0
}
>>;

using type = EVK::VertexShader<vertexFilename, EVK::NoPushConstants, Attributes,
EVK::UBOUniform<0, 0, UBO_Global>
>;

static_assert(EVK::vertexShader_c<type>);

} // namespace VertexShader

namespace FragmentShader {

static constexpr char fragmentFilename[] = "../Resources/Shaders/fragSkybox.spv";

using type = EVK::Shader<VK_SHADER_STAGE_FRAGMENT_BIT, fragmentFilename, EVK::NoPushConstants,
EVK::UBOUniform<0, 0, UBO_Global>,
EVK::CombinedImageSamplersUniform<0, 1, 1>
>;
static_assert(EVK::shader_c<type>);

} // namespace FragmentShader

using type = EVK::RenderPipeline<VertexShader::type, FragmentShader::type>;

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

} // namespace PipelineSkybox




//struct Pipeline_Skybox {
//	static const int descriptorSetsN = 1;
//	
//	static const int cubemapBinding = 1;
//	
//	static const int ubosN = 1;
//	
//	struct UBO_Global {
//		mat<4, 4, float32_t> viewInv;
//		mat<4, 4, float32_t> proj;
//		vec<4, float32_t> cameraPosition; // only using first three components
//		
//		static const int binding = 0;
//	};
//	
//	struct Attributes {
//		struct Vertex {
//			vec<3, float32_t> position;
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
//					12,//sizeof(Vertex), // stride
//					VK_VERTEX_INPUT_RATE_VERTEX // input rate
//				}
//			},
//			1,
//			(VkVertexInputAttributeDescription[1]){
//				{
//					0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0//offsetof(Vertex, position)
//				}
//			}
//		};
//	};
//};


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

std::shared_ptr<type> Build(std::shared_ptr<EVK::Devices> devices, VkRenderPass renderPassHandle);

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

struct Pipeline_Histogram {
	struct UBO {
		float32_t params[4];
		
		static const int binding = 0;
	};
	static const int ubosN = 1;
	
	struct SBO {
		uint32_t bin[256];
		
		static const int binding = 2;
	};
	
	static const int hdrImageBinding = 1;
};

// Global constants
struct Globals {
	struct MainInstanced {
		static constexpr int vertexVBIndexOffset = 0;									// first vertex buffer index for per-vertex data
		static const int renderedN = 1; // note that for instanced rendering, each object gets two vertex buffers: one for per-vertex data and one for per-devices.instance data
		
		static constexpr int IBIndexOffset = 0;											// first index buffer index
		static const int indexedN = 0;													// number of objects that are rendered indexed (each of which gets an index buffer)
	};
	struct MainOnce {
		static constexpr int vertexVBIndexOffset = MainInstanced::vertexVBIndexOffset + 2*MainInstanced::renderedN; // the two is because the instanced rendering in 'MainInstanced' requires 2 vertex buffers per object
		static const int renderedN = 3;
		
		static constexpr int IBIndexOffset = MainInstanced::IBIndexOffset + MainInstanced::indexedN;
		static const int indexedN = 0;
	};
	struct HUD {
		static constexpr int vertexVBIndexOffset = MainOnce::vertexVBIndexOffset + MainOnce::renderedN;
		static const int renderedN = 1;
		
		static constexpr int IBIndexOffset = MainOnce::IBIndexOffset + MainOnce::indexedN;
		static const int indexedN = 1;
	};
	struct Skybox {
		static constexpr int vertexVBIndexOffset = HUD::vertexVBIndexOffset + HUD::renderedN;
		static const int renderedN = 1;
		
		static constexpr int IBIndexOffset = HUD::IBIndexOffset + HUD::indexedN;
		static const int indexedN = 1;
	};
	struct Finall {
		static constexpr int vertexVBIndexOffset = Skybox::vertexVBIndexOffset + Skybox::renderedN;
		static const int renderedN = 1;
		
		static constexpr int IBIndexOffset = Skybox::IBIndexOffset + Skybox::indexedN;
		static const int indexedN = 1;
	};
	
//	static constexpr int vertexBuffersN = 2*MainInstanced::renderedN + + MainOnce::renderedN + HUD::renderedN + Skybox::renderedN + Finall::renderedN;	// total number of vertex buffers required (for both per-vertex and per-devices.instance data, across all pipelines)
//	static constexpr int indexBuffersN = MainInstanced::indexedN + MainOnce::indexedN + HUD::indexedN + Skybox::indexedN + Finall::indexedN;											// total number of index buffers required (across all pipelines)
//	static constexpr int ubosN = Shared_Main::ubosN + Pipeline_Hud::ubosN + Shared_Shadow::ubosN + Pipeline_Skybox::ubosN + Pipeline_Histogram::ubosN;										// total number of uniform buffer objects required (across all pipelines)
	
	static constexpr vec<3> lightDirection = (vec<3>){-0.700140042f, 0.1400280084f, -0.700140042f};
	static constexpr float cameraZNear = 0.1f;
	static constexpr float cameraZFar = 1000.0f;
	static constexpr float cascadeSplitLambda = 0.5f;
};

#endif /* Header_hpp */

