#ifndef Header_hpp
#define Header_hpp

#include <map>
#include <sys/time.h>

#include <mattresses.h>
#include <ESDL/ESDL_General.h>
#include <ESDL/ESDL_EventHandler.h>
#include <SDL2/SDL_vulkan.h>
#include <evk/ShaderProgram.hpp>
#include <evk/Resources.hpp>

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

