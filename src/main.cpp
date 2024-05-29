#include <evk/Interface.hpp>

#include "Header.hpp"
#include "RenderObjects.hpp"
#include "Pipelines.hpp"
#include "PipelineHud.hpp"
#include "PipelineMain.hpp"
#include "PipelineShadow.hpp"
#include "PipelineSkybox.hpp"
#include "PipelineFinal.hpp"
#include "CascadedShadowMap.hpp"

const int Globals::MainInstanced::renderedN;
const int Globals::MainOnce::renderedN;


static const int planeSize = 1000.0f;
static const int planeTextureSize = 100.0f;
static const float planeTextureRepeats = planeSize/planeTextureSize;
static ObjectData planeData = {
	6,
	(float32_t[]){
		-planeSize,-planeSize, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 planeSize, planeSize, 0.0f, 0.0f, 0.0f, 1.0f, planeTextureRepeats, planeTextureRepeats,
		-planeSize, planeSize, 0.0f, 0.0f, 0.0f, 1.0f, planeTextureRepeats, 0.0f,
		-planeSize,-planeSize, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 planeSize,-planeSize, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, planeTextureRepeats,
		 planeSize, planeSize, 0.0f, 0.0f, 0.0f, 1.0f, planeTextureRepeats, planeTextureRepeats
	},
	(ObjectDivisionData[]){
		{
			0, 6, 0
		}
	},
	1
};

static const uint32_t hudVerticesN = 4;
static const PipelineHud::Vertex hudVertices[hudVerticesN] = {
	{{   0.0f,  0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f}},
	{{   0.0f,-32.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f}},
	{{-128.0f,-32.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f}},
	{{-128.0f,  0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f}}
};
static const uint32_t hudIndicesN = 6;
static const uint32_t hudIndices[hudIndicesN] = {
	0, 1, 2,
	2, 3, 0
};

static const uint32_t finalVerticesN = 4;
static const PipelineFinal::Vertex finalVertices[finalVerticesN] = {
	{{-1.0f,-1.0f}, { 0.0f, 0.0f}},
	{{ 1.0f,-1.0f}, { 1.0f, 0.0f}},
	{{ 1.0f, 1.0f}, { 1.0f, 1.0f}},
	{{-1.0f, 1.0f}, { 0.0f, 1.0f}}
};
static const uint32_t finalIndicesN = 6;
static const uint32_t finalIndices[finalIndicesN] = {
	0, 1, 2,
	2, 3, 0
};

static const uint32_t skyboxVerticesN = 8;
static const PipelineSkybox::Vertex skyboxVertices[skyboxVerticesN] = {
	{ 1.0f, 1.0f, 1.0f},
	{ 1.0f, 1.0f,-1.0f},
	{ 1.0f,-1.0f,-1.0f},
	{ 1.0f,-1.0f, 1.0f},
	{-1.0f, 1.0f, 1.0f},
	{-1.0f, 1.0f,-1.0f},
	{-1.0f,-1.0f,-1.0f},
	{-1.0f,-1.0f, 1.0f}
};
static const uint32_t skyboxIndicesN = 36;
static const uint32_t skyboxIndices[skyboxIndicesN] = {
	0, 1, 2, // x+
	0, 2, 3,
	4, 5, 1, // y+
	4, 1, 0,
	7, 6, 5, // x-
	7, 5, 4,
	3, 2, 6, // y-
	3, 6, 7,
	2, 1, 5, // z-
	2, 5, 6,
	4, 0, 3, // z+
	4, 3, 7
};


// devices declared first as we need it to be destroyed last
std::shared_ptr<EVK::Devices> devices {};
std::shared_ptr<EVK::Interface> interface {};

int imageIndex = OTHER_IMAGES_N;

std::map<std::string, uint32_t> materialDictionary = {};
std::array<std::shared_ptr<EVK::TextureImage>, PNGS_N> pngsArray; // a texture ID is the index in this array at which the texture image index (in the `Vulkan` devices.instance) is stored
int pngIndexIndex = 0;
uint32_t GetTextureIdFromMtl(const char *usemtl){
	if(materialDictionary.contains(std::string(usemtl))) return materialDictionary[std::string(usemtl)];
	
	const int nameLength = (int)strlen(usemtl);
	char buffer[512];
	static const char *prefix = "../Resources/Textures/";
	static const int prefixLength = (int)strlen(prefix);
	static const char *suffix = ".png";
	static const int suffixLength = (int)(strlen(suffix) + 1); // includes null terminator
	memcpy(buffer, prefix, prefixLength*sizeof(char));
	memcpy(buffer + prefixLength, usemtl, nameLength*sizeof(char));
	memcpy(buffer + prefixLength + nameLength, suffix, suffixLength*sizeof(char));
	pngsArray[pngIndexIndex] = std::make_shared<EVK::TextureImage>(devices, EVK::PNGImageBlueprint{buffer});
	materialDictionary[std::string(usemtl)] = pngIndexIndex;
	return pngIndexIndex++;
}

#define OBJ_DATAS_N 4
enum class ObjData {player, chair, chainsaw, plane};
ObjectData objDatas[OBJ_DATAS_N];

class Player : public Rendered::Once {
public:
	Player(std::shared_ptr<EVK::Devices> _devices, const vec<2> &_position) : Rendered::Once(_devices, objDatas[(int)ObjData::player]/*ReadProcessedOBJFile("ProcessedObjFiles/MaleLow.bin", &GetTextureIdFromMtl)*/), position(_position | 50.0f) {
		SDL_Event event;
		event.type = SDL_MOUSEMOTION;
		mmEID = ESDL::AddEventCallback((MemberFunction<Player, void, SDL_Event>){this, &Player::MouseMoved}, event);
		
		cursorX = cursorY = 0;
	}
	~Player(){
		ESDL::RemoveEventCallback(mmEID);
	}
	
	void Update(float dT, PerObject *perObjectDataPtr) override {
		yaw = -yawSensitivity*(float)cursorX;
		pitch = -pitchSensitivity*(float)cursorY;
		if(pitch > 0.5f*M_PI) pitch = 0.5f*M_PI;
		else if(pitch < -0.5f*M_PI) pitch = -0.5f*M_PI;
		
		const float cy = cosf(yaw);
		const float sy = sinf(yaw);
		const float forward = (float)(ESDL::GetKeyDown(SDLK_w) - ESDL::GetKeyDown(SDLK_s));
		const float right = (float)(ESDL::GetKeyDown(SDLK_d) - ESDL::GetKeyDown(SDLK_a));
		position += speed*dT*(vec<3>){forward*cy + right*sy, forward*sy - right*cy, (float)(ESDL::GetKeyDown(SDLK_SPACE) - ESDL::GetKeyDown(SDLK_LSHIFT))};
		
		perObjectDataPtr->model = mat<4, 4>::Translation((vec<2>){position.x, position.y} | 0.0f) &
								  mat<4, 4>::ZRotation(0.5f * float(M_PI) + yaw) &
								  mat<4, 4>::XRotation(0.5f * float(M_PI)) &
								  mat<4, 4>::Scaling({2.0f, 2.0f, 2.0f});
		
		perObjectDataPtr->modelInvT = perObjectDataPtr->model.Inverted().Transposed();
	}
	
	void MouseMoved(SDL_Event event){
		cursorX += event.motion.xrel;
		cursorY += event.motion.yrel;
	}
	
	const mat<4, 4> &GetViewInverseMatrix(){
		viewInverseMatrix = (mat<4, 4>::Translation(position) &
							 mat<4, 4>::ZRotation(-0.5f * float(M_PI) + yaw) &
							 mat<4, 4>::XRotation(0.5f * float(M_PI) + pitch)).Inverted();
		return viewInverseMatrix;
	}
	
	const vec<3> &GetCameraPosition(){ return position; }
	
private:
	mat<4, 4> viewInverseMatrix;
	
	vec<3> position;
	float speed = 200.0f;
	int cursorX, cursorY;
	float yaw;
	float pitch;
	float yawSensitivity = 0.002f;
	float pitchSensitivity = 0.002f;
	
	int mmEID;
};

class ChairInstanceManager : public Rendered::InstanceManager {
public:
	ChairInstanceManager(std::shared_ptr<EVK::Devices> _devices) : Rendered::InstanceManager(_devices, objDatas[(int)ObjData::chair]/*ReadProcessedOBJFile("ProcessedObjFiles/chair.bin", &GetTextureIdFromMtl)*/){}
	
	Rendered::Info Render(VkCommandBuffer commandBuffer) override {
		Rendered::Info ret = Rendered::InstanceManager::Render(commandBuffer);
		ret.shininess = 1.0f;
		return ret;
	}
};
class ChairInstance : public Rendered::Instance {
public:
	ChairInstance(ChairInstanceManager *_manager) : Rendered::Instance(_manager) {
		position = {0.0f, 0.0f};
		angle = 0.0f;
	}
	
	void Update(float dT, PerObject *perObjectDataPtr) override {
		
		perObjectDataPtr->model = (mat<4, 4>::Translation(position | 0.0f) &
								   mat<4, 4>::ZRotation(angle) &
								   mat<4, 4>::XRotation(0.5f * float(M_PI)));
		
		perObjectDataPtr->modelInvT = perObjectDataPtr->model.Inverted().Transposed();
		
		savedInstanceData = *perObjectDataPtr;
		
		angle += dT * spinSpeed;
		const int fb = ESDL::GetKeyDown(SDLK_UP) - ESDL::GetKeyDown(SDLK_DOWN);
		const int rl = ESDL::GetKeyDown(SDLK_RIGHT) - ESDL::GetKeyDown(SDLK_LEFT);
		vec<2> vel = speed*(float(fb) * (vec<2>){-1.0f, 0.0f} + float(rl) * (vec<2>){0.0f, 1.0f});
		if(fb && rl) vel *= 0.70712f;
		position += vel * dT;
	}
	
	const PerObject &GetInstanceData() const { return savedInstanceData; }
	
private:
	vec<2> position;
	float angle;
	float spinSpeed = 1.0f;
	float speed = 100.0f;
	
	PerObject savedInstanceData;
};


class ChainSaw : public Rendered::Once {
public:
	ChainSaw(std::shared_ptr<EVK::Devices> _devices, ChairInstance *_chair) : Rendered::Once(_devices, objDatas[(int)ObjData::chainsaw]/*ReadProcessedOBJFile("ProcessedObjFiles/chainsaw.bin", &GetTextureIdFromMtl)*/), chair(_chair) {}
	
	void Update(float dT, PerObject *perObjectDataPtr) override {
		*perObjectDataPtr = chair->GetInstanceData();
		
		perObjectDataPtr->model = (perObjectDataPtr->model &
								   mat<4, 4>::Translation({-5.0f, 27.0f, 0.0f}) &
								   mat<4, 4>::Scaling({5.0f, 5.0f, 5.0f}) &
								   mat<4, 4>::ZRotation(-0.5f * float(M_PI)) &
								   mat<4, 4>::XRotation(0.5f * float(M_PI)));
		
		perObjectDataPtr->modelInvT = perObjectDataPtr->model.Inverted().Transposed();
	}
	
	Rendered::Info Render(VkCommandBuffer commandBuffer) override {
		Rendered::Info ret = Rendered::Once::Render(commandBuffer);
		ret.shininess = 100.0f;
		return ret;
	}
	
private:
	ChairInstance *chair;
};

class Plane : public Rendered::Once {
public:
	Plane(std::shared_ptr<EVK::Devices> _devices) : Rendered::Once(_devices, objDatas[(int)ObjData::plane]/*planeData*/) {
		
	}
	
	void Update(float dT, PerObject *perObjectData) override {
		perObjectData->model = mat<4, 4, float32_t>::Identity();
		perObjectData->modelInvT = mat<4, 4, float32_t>::Identity();
	}
	
	Rendered::Info Render(VkCommandBuffer commandBuffer) override {
		Rendered::Info ret = Rendered::Once::Render(commandBuffer);
		ret.shininess = 1.0f;
		return ret;
	}
};

// pipelines
std::shared_ptr<PipelineMain::Instanced::type> pipelineMainInstanced;
std::shared_ptr<PipelineMain::Once::type> pipelineMainOnce;
std::shared_ptr<PipelineHud::type> pipelineHud;
std::shared_ptr<PipelineShadow::Instanced::type> pipelineShadowInstanced;
std::shared_ptr<PipelineShadow::Once::type> pipelineShadowOnce;
std::shared_ptr<PipelineSkybox::type> pipelineSkybox;
std::shared_ptr<PipelineFinal::type> pipelineFinal;

// UBOs
std::shared_ptr<EVK::UniformBufferObject<PipelineMain::UBO_Global, false>> uboMainGlobal;
std::shared_ptr<EVK::UniformBufferObject<PerObject, true>> uboPerObject;
std::shared_ptr<EVK::UniformBufferObject<PipelineShadow::UBO_Global, false>> uboShadowGlobal;
std::shared_ptr<EVK::UniformBufferObject<PipelineHud::UBO, false>> uboHud;
std::shared_ptr<EVK::UniformBufferObject<PipelineSkybox::UBO_Global, false>> uboSkyboxGlobal;

// VBOs & IBOs
std::shared_ptr<EVK::VertexBufferObject> vboHud;
std::shared_ptr<EVK::IndexBufferObject> iboHud;
std::shared_ptr<EVK::VertexBufferObject> vboSkybox;
std::shared_ptr<EVK::IndexBufferObject> iboSkybox;
std::shared_ptr<EVK::VertexBufferObject> vboFinal;
std::shared_ptr<EVK::IndexBufferObject> iboFinal;

Rendered::InstanceManager *renderedInstanced[Globals::MainInstanced::renderedN];
Rendered::Once *renderedOnce[Globals::MainOnce::renderedN];
Player *player;

void Update(uint32_t flight, float dT, Shared_Main::PushConstants_Vert &vertPcs, Shared_Main::PushConstants_Frag &fragPcs){
	
	// UBOs
	PipelineMain::UBO_Global *const uboGlobalPointer = uboMainGlobal->GetDataPointer(flight);
	std::vector<PerObject *> uboPerObjectPointers = uboPerObject->GetDataPointers(flight);
	PipelineShadow::UBO_Global *const uboShadowPointer = uboShadowGlobal->GetDataPointer(flight);
	PipelineSkybox::UBO_Global *const uboSkyboxPointer = uboSkyboxGlobal->GetDataPointer(flight);
	PipelineHud::UBO *const uboHudPointer = uboHud->GetDataPointer(flight);
	
	// Updating
	for(int i=0; i<Globals::MainInstanced::renderedN; i++) renderedInstanced[i]->Update(dT);
	for(int i=0; i<Globals::MainOnce::renderedN; i++) renderedOnce[i]->Update(dT, uboPerObjectPointers[i]);
	
	// Setting main global UBO
	uboGlobalPointer->viewInv = player->GetViewInverseMatrix();
	uboGlobalPointer->proj = mat<4, 4>::PerspectiveProjection(M_PI*0.25f, (float)interface->GetExtentWidth() / (float)interface->GetExtentHeight(), Globals::cameraZNear, Globals::cameraZFar);
	uboGlobalPointer->proj[1][1] *= -1.0f;
	uboGlobalPointer->lightDir.xyz_r() = Globals::lightDirection;
	const vec<4> sunColour = {0.9882352941f, 0.8980392157f, 0.4392156863f, 1.0f};
	uboGlobalPointer->lightColour = sunColour;
	uboGlobalPointer->cameraPosition = player->GetCameraPosition() | 1.0f;
	
	// Setting shadow UBO and main lightMats
	UpdateCascades(uboGlobalPointer, uboShadowPointer);
	
	// Setting skybox UBO
	uboSkyboxPointer->viewInv = uboGlobalPointer->viewInv;
	uboSkyboxPointer->viewInv[3][0] = uboSkyboxPointer->viewInv[3][1] = uboSkyboxPointer->viewInv[3][2] = 0.0f; // removing translational component
	uboSkyboxPointer->proj = uboGlobalPointer->proj;
	uboSkyboxPointer->proj[2][2] = -1.0f; uboSkyboxPointer->proj[3][2] = 0.0f; // fix the depth at 1.0
	uboSkyboxPointer->cameraPosition = player->GetCameraPosition() | 1.0f;
	
	// vertex shader push constants
	//currently placeholder
	
	// fragment shader push constants
	const vec<4> white = {1.0f, 1.0f, 1.0f, 1.0f};
	fragPcs.colourMult = white;
	fragPcs.specular = white;
	fragPcs.shininess = 100.0f;
	fragPcs.specularFactor = 0.05f;
	
	// Setting HUD UBO
	*uboHudPointer = {(float32_t)interface->GetExtentWidth(), (float32_t)interface->GetExtentHeight(), 30.0f};
}

void RenderShadowMap(VkCommandBuffer commandBuffer, uint32_t flight, Shared_Main::PushConstants_Vert shadPcs, int cascadeLayer){
	shadPcs.cascadeLayer = cascadeLayer;
	
	pipelineShadowInstanced->CmdBind(commandBuffer);
	if(pipelineShadowInstanced->CmdBindDescriptorSets<0, 0>(commandBuffer, flight)){
		pipelineShadowInstanced->CmdPushConstants<0>(commandBuffer, &shadPcs);
		for(int i=0; i<Globals::MainInstanced::renderedN; ++i){
			Rendered::Info info = renderedInstanced[i]->Render(commandBuffer);
			for(int j=0; j<info.n; ++j){
				Rendered::Info::Draw draw = info.drawFunction(j);
				interface->CmdDraw(draw.vertexCount, draw.instanceCount, draw.firstVertex, draw.firstInstance);
			}
		}
	}
	
	pipelineShadowOnce->CmdBind(commandBuffer);
	pipelineShadowOnce->CmdPushConstants<0>(commandBuffer, &shadPcs);
	std::vector<int> indices(1);
	for(int i=0; i<Globals::MainOnce::renderedN; ++i){
		indices[0] = i;
		if(pipelineShadowOnce->CmdBindDescriptorSets<0, 0>(commandBuffer, flight, indices)){
		   Rendered::Info info = renderedOnce[i]->Render(commandBuffer);
		   for(int j=0; j<info.n; ++j){
			   Rendered::Info::Draw draw = info.drawFunction(j);
			   interface->CmdDraw(draw.vertexCount, draw.instanceCount, draw.firstVertex, draw.firstInstance);
		   }
		}
	}
}

void RenderScene(VkCommandBuffer commandBuffer, uint32_t flight, Shared_Main::PushConstants_Vert vertPcs, Shared_Main::PushConstants_Frag fragPcs){
	pipelineMainInstanced->CmdBind(commandBuffer);
	if(pipelineMainInstanced->CmdBindDescriptorSets<0, 0>(commandBuffer, flight)){
		for(int i=0; i<Globals::MainInstanced::renderedN; ++i){
			Rendered::Info info = renderedInstanced[i]->Render(commandBuffer);
			fragPcs.shininess = info.shininess;
			for(int j=0; j<info.n; ++j){
				Rendered::Info::Draw draw = info.drawFunction(j);
				fragPcs.textureID = draw.textureId;
				pipelineMainInstanced->CmdPushConstants<0>(commandBuffer, &fragPcs);
				interface->CmdDraw(draw.vertexCount, draw.instanceCount, draw.firstVertex, draw.firstInstance);
			}
		}
	} else {
		std::cout << "Failed to draw main instanced.\n";
	}
	
	pipelineMainOnce->CmdBind(commandBuffer);
	std::vector<int> indices(1);
	for(int i=0; i<Globals::MainOnce::renderedN; ++i){
		indices[0] = i;
		if(pipelineMainOnce->CmdBindDescriptorSets<0, 0>(commandBuffer, flight, indices)){
			Rendered::Info info = renderedOnce[i]->Render(commandBuffer);
			fragPcs.shininess = info.shininess;
			for(int j=0; j<info.n; ++j){
				Rendered::Info::Draw draw = info.drawFunction(j);
				fragPcs.textureID = draw.textureId;
				pipelineMainOnce->CmdPushConstants<0>(commandBuffer, &fragPcs);
				interface->CmdDraw(draw.vertexCount, draw.instanceCount, draw.firstVertex, draw.firstInstance);
			}
		} else {
			std::cout << "Failed to draw main onces.\n";
		}
	}
}

void RenderHUD(VkCommandBuffer commandBuffer, uint32_t flight){
	pipelineHud->CmdBind(commandBuffer);
	if(pipelineHud->CmdBindDescriptorSets<0, 0>(commandBuffer, flight) &&
	   vboHud->CmdBind(commandBuffer, 0) &&
	   iboHud->CmdBind(commandBuffer, VK_INDEX_TYPE_UINT32)){
		interface->CmdDrawIndexed(iboHud->GetIndexCount().value());
	} else {
		std::cout << "Failed to render HUD.\n";
	}
}

struct CallbackReceiver {
	CallbackReceiver(std::shared_ptr<EVK::Interface> _ir) : ir(_ir) {}
	
	void ResizeCallback(SDL_Event event){
		ir->FramebufferResizeCallback();
	}
	
private:;
	std::shared_ptr<EVK::Interface> ir;
};

std::shared_ptr<EVK::TextureImage> otherColourImage;
std::shared_ptr<EVK::TextureImage> otherDepthImage;
std::array<std::shared_ptr<EVK::TextureSampler>, int(Sampler::_COUNT_)> samplers;

void CreateOtherImages(const vec<2, uint32_t> &size){
	VkImageCreateInfo imageCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = {
			.width = size.x,
			.height = size.y,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = FINAL_FORMAT,
		.tiling = VK_IMAGE_TILING_OPTIMAL, // VK_IMAGE_TILING_LINEAR for row-major order if we want to access texels in the memory of the image
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
#ifdef MSAA
		.samples = devices.GetMSAASamples(),
#else
		.samples = VK_SAMPLE_COUNT_1_BIT,
#endif
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	otherColourImage = std::make_shared<EVK::TextureImage>(devices, EVK::ManualImageBlueprint{imageCI, VK_IMAGE_VIEW_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT});
	
	imageCI.format = devices->FindDepthFormat();
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	otherDepthImage = std::make_shared<EVK::TextureImage>(devices, EVK::ManualImageBlueprint{imageCI, VK_IMAGE_VIEW_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT});
}

std::shared_ptr<EVK::BufferedRenderPass> finalRenderPass;

void ResizeCallback(const vec<2, uint32_t> &size){
	CreateOtherImages(size);
	finalRenderPass->SetImages({otherColourImage, otherDepthImage});
	pipelineFinal->iDescriptorSet<0>().iDescriptor<0>().Set({{{otherColourImage, samplers[int(Sampler::main)]}}});
};

int main(int argc, const char * argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	ESDL::InitEventHandler();
	
	SDL_Window *window = SDL_CreateWindow("VulkanFirst", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	uint32_t requiredExtensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount, nullptr);
	std::vector<const char *> requiredExtensions(requiredExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &requiredExtensionCount, requiredExtensions.data());
	devices = std::make_shared<EVK::Devices>("VulkanFirst", requiredExtensions,
										[window](VkInstance instance) -> VkSurfaceKHR {
		VkSurfaceKHR ret;
		if(SDL_Vulkan_CreateSurface(window, instance, &ret) == SDL_FALSE)
			throw std::runtime_error("failed to create window devices.surface!");
		return ret;
	},
										[window]() -> VkExtent2D {
		int width, height;
		SDL_GL_GetDrawableSize(window, &width, &height);
		return (VkExtent2D){
			.width = uint32_t(width),
			.height = uint32_t(height)
		};
	});
	
	
	std::shared_ptr<EVK::LayeredBufferedRenderPass<SHADOW_MAP_CASCADE_COUNT>> shadowMapRenderPass = BuildShadowMapRenderPass(devices);

	finalRenderPass = BuildFinalRenderPass(devices);
	
	interface = std::make_shared<EVK::Interface>(devices);
	
	interface->SetResizeCallback(&ResizeCallback);
	
	pipelineMainInstanced = PipelineMain::Instanced::Build(devices, finalRenderPass->RenderPassHandle());
	pipelineMainOnce = PipelineMain::Once::Build(devices, finalRenderPass->RenderPassHandle());
	pipelineHud = PipelineHud::Build(devices, finalRenderPass->RenderPassHandle());
	pipelineShadowInstanced = PipelineShadow::Instanced::Build(devices, shadowMapRenderPass->RenderPassHandle());
	pipelineShadowOnce = PipelineShadow::Once::Build(devices, shadowMapRenderPass->RenderPassHandle());
	pipelineSkybox = PipelineSkybox::Build(devices, finalRenderPass->RenderPassHandle());
	pipelineFinal = PipelineFinal::Build(devices, interface->GetRenderPassHandle());

	uboMainGlobal = std::make_shared<EVK::UniformBufferObject<PipelineMain::UBO_Global, false>>(devices);
	uboPerObject = std::make_shared<EVK::UniformBufferObject<PerObject, true>>(devices, Globals::MainOnce::renderedN);
	uboShadowGlobal = std::make_shared<EVK::UniformBufferObject<PipelineShadow::UBO_Global, false>>(devices);
	uboHud = std::make_shared<EVK::UniformBufferObject<PipelineHud::UBO, false>>(devices);
	uboSkyboxGlobal = std::make_shared<EVK::UniformBufferObject<PipelineSkybox::UBO_Global, false>>(devices);
	
//	vulkan = NewBuildPipelines(devices);
	
	samplers = BuildSamplers(devices);
	
	CallbackReceiver cr {interface};
	SDL_Event event;
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
	ESDL::AddEventCallback((MemberFunction<CallbackReceiver, void, SDL_Event>){&cr, &CallbackReceiver::ResizeCallback}, event);
	
	std::shared_ptr<EVK::TextureImage> cubemapImage;
	{ // preparing skybox
		std::array<std::string, 6> cubemapFiles = {{
			"../Resources/textures/skybox_b.png", // correct
			"../Resources/textures/skybox_e.png", // correct
			"../Resources/textures/skybox_d.png", // correct
			"../Resources/textures/skybox_f.png", // correct
			"../Resources/textures/skybox_a.png", // correct
			"../Resources/textures/skybox_c.png" // correct
		}};
		cubemapImage = std::make_shared<EVK::TextureImage>(devices, EVK::CubemapPNGImageBlueprint{cubemapFiles});
	}
	
	std::shared_ptr<EVK::TextureImage> shadowCascades;
	{
		// For shadow mapping we only need a depth attachment
		VkImageCreateInfo imageCI = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.extent = {
				.width = SHADOWMAP_DIM,
				.height = SHADOWMAP_DIM,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = SHADOW_MAP_CASCADE_COUNT,
			.format = DEPTH_FORMAT,
			.tiling = VK_IMAGE_TILING_OPTIMAL, // VK_IMAGE_TILING_LINEAR for row-major order if we want to access texels in the memory of the image
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};
		shadowCascades = std::make_shared<EVK::TextureImage>(devices, EVK::ManualImageBlueprint{imageCI, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT});
	}
	
	
	
	objDatas[(int)ObjData::player] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/MaleLow.bin", &GetTextureIdFromMtl);
	objDatas[(int)ObjData::chair] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/chair.bin", &GetTextureIdFromMtl);
	objDatas[(int)ObjData::chainsaw] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/chainsaw.bin", &GetTextureIdFromMtl);
	planeData.divisionData[0].texture = GetTextureIdFromMtl("concrete-917");
	objDatas[(int)ObjData::plane] = planeData;
	
	
//	BuildVkInterfaceStructures(vulkan, pngsIndexArray);
	
	// updating descriptor sets
//	for(int i=0; i<GRAPHICS_PIPELINES_N; ++i) vulkan->GP(i).UpdateDescriptorSets();
//	vulkan->CP(0).UpdateDescriptorSets();
	
	pipelineMainInstanced->iDescriptorSet<0>().iDescriptor<0>().Set(uboMainGlobal);
	pipelineMainInstanced->iDescriptorSet<0>().iDescriptor<1>().Set({{samplers[int(Sampler::main)]}});
	pipelineMainInstanced->iDescriptorSet<0>().iDescriptor<2>().Set(pngsArray);
	pipelineMainInstanced->iDescriptorSet<0>().iDescriptor<3>().Set({{shadowCascades, samplers[int(Sampler::shadow)]}});
	
	pipelineMainOnce->iDescriptorSet<0>().iDescriptor<0>().Set(uboMainGlobal);
	pipelineMainOnce->iDescriptorSet<1>().iDescriptor<0>().Set(uboPerObject);
	pipelineMainOnce->iDescriptorSet<0>().iDescriptor<1>().Set({{samplers[int(Sampler::main)]}});
	pipelineMainOnce->iDescriptorSet<0>().iDescriptor<2>().Set(pngsArray);
	pipelineMainOnce->iDescriptorSet<0>().iDescriptor<3>().Set({{shadowCascades, samplers[int(Sampler::shadow)]}});
	
	pipelineShadowInstanced->iDescriptorSet<0>().iDescriptor<0>().Set(uboShadowGlobal);
	
	pipelineShadowOnce->iDescriptorSet<0>().iDescriptor<0>().Set(uboShadowGlobal);
	pipelineShadowOnce->iDescriptorSet<1>().iDescriptor<0>().Set(uboPerObject);
	
	pipelineSkybox->iDescriptorSet<0>().iDescriptor<0>().Set(uboSkyboxGlobal);
	pipelineSkybox->iDescriptorSet<0>().iDescriptor<1>().Set({{{cubemapImage, samplers[int(Sampler::cube)]}}});
	
	pipelineHud->iDescriptorSet<0>().iDescriptor<1>().Set(uboHud);
	pipelineHud->iDescriptorSet<0>().iDescriptor<0>().Set({{samplers[int(Sampler::main)]}});
	pipelineHud->iDescriptorSet<0>().iDescriptor<2>().Set({{pngsArray[0]}});
	
	// updated buffered render pass
//	vulkan->UpdateLayeredBufferedRenderPass(0);
	shadowMapRenderPass->SetImage(shadowCascades);
	
	vboHud = std::make_shared<EVK::VertexBufferObject>(devices);
	vboHud->Fill((void *)hudVertices, sizeof(hudVertices));
	iboHud = std::make_shared<EVK::IndexBufferObject>(devices);
	iboHud->Fill((uint32_t *)hudIndices, hudIndicesN);
	
	vboSkybox = std::make_shared<EVK::VertexBufferObject>(devices);
	vboSkybox->Fill((void *)skyboxVertices, sizeof(skyboxVertices));
	iboSkybox = std::make_shared<EVK::IndexBufferObject>(devices);
	iboSkybox->Fill((uint32_t *)skyboxIndices, skyboxIndicesN);
	
	vboFinal = std::make_shared<EVK::VertexBufferObject>(devices);
	vboFinal->Fill((void *)finalVertices, sizeof(finalVertices));
	iboFinal = std::make_shared<EVK::IndexBufferObject>(devices);
	iboFinal->Fill((uint32_t *)finalIndices, finalIndicesN);
	
	ChairInstanceManager *chairManager;
	ChairInstance *chair;
	ChainSaw *chainSaw;
	Plane *plane;
	
	renderedInstanced[0] = chairManager = new ChairInstanceManager(devices);
	chair = new ChairInstance(chairManager);
	
	renderedOnce[0] = chainSaw = new ChainSaw(devices, chair);
	renderedOnce[1] = plane = new Plane(devices);
	renderedOnce[2] = player = new Player(devices, {100.0f, 0.0f});
	
	int time = SDL_GetTicks();
	
	while(!ESDL::HandleEvents()){
		
		const int newTime = SDL_GetTicks();
		const float dT = 0.001f*(float)(newTime - time);
		time = newTime;
		
		Shared_Main::PushConstants_Vert vertPcs;
		Shared_Main::PushConstants_Frag fragPcs;
		
		std::vector<VkClearValue> clearVals = {
			{
				.color = {{1.0f, 1.0f, 1.0f, 1.0f}}
			},
			{
				.depthStencil = {1.0f, 0}
			}
		};
		
		
		if(std::optional<EVK::Interface::FrameInfo> fi = interface->BeginFrame(); fi.has_value()){
			
			Update(fi->frame, dT, vertPcs, fragPcs);
			
			for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; i++){
				if(shadowMapRenderPass->CmdBegin(fi->cb, fi->frame, VK_SUBPASS_CONTENTS_INLINE, {clearVals[1]}, i)){
					//vulkan->CmdSetDepthBias(1.25f, 0.0f, 1.75f); // 1.25, 0.0, 1.75
					 RenderShadowMap(fi->cb, fi->frame, vertPcs, i);
					 interface->CmdEndRenderPass();
				}
			}
			
			/*
			vulkan->BeginFinalRenderPass();
			
			vulkan->GP((int)GraphicsPipeline::skybox).Bind();
			vulkan->GP((int)GraphicsPipeline::skybox).BindDescriptorSets(0, Pipeline_Skybox::descriptorSetsN);
			vulkan->CmdBindVertexBuffer(0, Globals::Skybox::vertexVBIndexOffset);
			vulkan->CmdBindIndexBuffer(Globals::Skybox::IBIndexOffset, VK_INDEX_TYPE_UINT32);
			vulkan->CmdDrawIndexed(vulkan->GetIndexBufferCount(Globals::Skybox::IBIndexOffset));
			
			RenderScene(vertPcs, fragPcs);
			
			RenderHUD();
			 */
			
			if(finalRenderPass->CmdBegin(fi->cb, fi->frame, VK_SUBPASS_CONTENTS_INLINE, clearVals)){
				pipelineSkybox->CmdBind(fi->cb);
				pipelineSkybox->CmdBindDescriptorSets<0, 0>(fi->cb, fi->frame);
				vboSkybox->CmdBind(fi->cb, 0);
				iboSkybox->CmdBind(fi->cb, VK_INDEX_TYPE_UINT32);
				interface->CmdDrawIndexed(iboSkybox->GetIndexCount().value());
				
				RenderScene(fi->cb, fi->frame, vertPcs, fragPcs);
				
				RenderHUD(fi->cb, fi->frame);
				
				interface->CmdEndRenderPass();
			}
			
			
			// compute commands recorded with a pipeline image memory barrier
			// begin compute command buffer
//			vulkan->BeginCompute();
//			vulkan->CmdPipelineImageMemoryBarrier(true, int(OtherImage::colour), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//			// submit to compute queue
//			vulkan->EndCompute();
			
			
			// final render pass recorded with a pipeline buffer memory barrier:
			interface->BeginFinalRenderPass({{1.0f, 1.0f, 1.0f, 1.0f}}); // begin with a pipeline buffer memory barrier
				
			pipelineFinal->CmdBind(fi->cb);
			pipelineFinal->CmdBindDescriptorSets<0, 0>(fi->cb, fi->frame);
			vboFinal->CmdBind(fi->cb, 0);
			iboFinal->CmdBind(fi->cb, VK_INDEX_TYPE_UINT32);
			interface->CmdDrawIndexed(iboFinal->GetIndexCount().value());
			
			//
			interface->EndFinalRenderPassAndFrame();
		}
	}
	
	SDL_DestroyWindow(window);
	
	free(objDatas[(int)ObjData::player].vertices);
	free(objDatas[(int)ObjData::chair].vertices);
	free(objDatas[(int)ObjData::chainsaw].vertices);
	for(int i=0; i<Globals::MainInstanced::renderedN; ++i) delete renderedInstanced[i];
	for(int i=0; i<Globals::MainOnce::renderedN; ++i) delete renderedOnce[i];
	delete player;
	
	return 0;
}
