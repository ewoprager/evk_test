#include "Header.hpp"
#include "RenderObjects.hpp"
#include "Pipelines.hpp"
#include "CascadedShadowMap.hpp"

// sometimes you need to add extra desclarations of static members:
const int Pipeline_Hud::descriptorSetsN;
const int Globals::MainInstanced::renderedN;
const int Globals::MainOnce::renderedN;
const int Pipeline_MainInstanced::descriptorSetsN;
const int Pipeline_MainOnce::descriptorSetsN;
const int Pipeline_ShadowOnce::descriptorSetsN;
const int Pipeline_ShadowInstanced::descriptorSetsN;
const int Pipeline_Skybox::descriptorSetsN;

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

static const int hudVerticesN = 4;
static const Pipeline_Hud::Attributes::Vertex hudVertices[hudVerticesN] = {
	{{   0.0f,  0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f}},
	{{   0.0f,-32.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f}},
	{{-128.0f,-32.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f}},
	{{-128.0f,  0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f}}
};
static const int hudIndicesN = 6;
static const uint32_t hudIndices[hudIndicesN] = {
	0, 1, 2,
	2, 3, 0
};

static const int finalVerticesN = 4;
static const Pipeline_Final::Attributes::Vertex finalVertices[finalVerticesN] = {
	{{-1.0f,-1.0f}, { 0.0f, 0.0f}},
	{{ 1.0f,-1.0f}, { 1.0f, 0.0f}},
	{{ 1.0f, 1.0f}, { 1.0f, 1.0f}},
	{{-1.0f, 1.0f}, { 0.0f, 1.0f}}
};
static const int finalIndicesN = 6;
static const uint32_t finalIndices[finalIndicesN] = {
	0, 1, 2,
	2, 3, 0
};

static const int skyboxVerticesN = 8;
static const Pipeline_Skybox::Attributes::Vertex skyboxVertices[skyboxVerticesN] = {
	{ 1.0f, 1.0f, 1.0f},
	{ 1.0f, 1.0f,-1.0f},
	{ 1.0f,-1.0f,-1.0f},
	{ 1.0f,-1.0f, 1.0f},
	{-1.0f, 1.0f, 1.0f},
	{-1.0f, 1.0f,-1.0f},
	{-1.0f,-1.0f,-1.0f},
	{-1.0f,-1.0f, 1.0f}
};
static const int skyboxIndicesN = 36;
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



EVK::Interface *vulkanPtr=nullptr;

EVK::ImageBlueprint *imageBlueprintPtrs[Globals::texturesN];
int imagesBlueprinted = 0;

#include <map>
std::map<std::string, uint32_t> materialDictionary = {};
int textureImageIndexArray[PNGS_N]; // a texture ID is the index in this array at which the texture image index (in the `Vulkan` devices.instance) is stored
int textureImageIndexCount = 0;
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
	imageBlueprintPtrs[imagesBlueprinted] = new EVK::PNGImageBlueprint(buffer);
	const int imageIndex = imagesBlueprinted++;
	textureImageIndexArray[textureImageIndexCount] = imageIndex;
	materialDictionary[std::string(usemtl)] = textureImageIndexCount;
	return textureImageIndexCount++;
}

#define OBJ_DATAS_N 4
enum class ObjData {player, chair, chainsaw, plane};
ObjectData objDatas[OBJ_DATAS_N];

class Player : public Rendered::Once {
public:
	Player(const int &_index, const vec2 &_position) : Rendered::Once(_index, objDatas[(int)ObjData::player]/*ReadProcessedOBJFile("ProcessedObjFiles/MaleLow.bin", &GetTextureIdFromMtl)*/), position(_position | 50.0f) {
		SDL_Event event;
		event.type = SDL_MOUSEMOTION;
		mmEID = ESDL::AddEventCallback((MemberFunction<Player, void, SDL_Event>){this, &Player::MouseMoved}, event);
		
		cursorX = cursorY = 0;
	}
	~Player(){
		ESDL::RemoveEventCallback(mmEID);
	}
	
	void Update(const float &dT, Shared_Main::PerObject *const &perObjectDataPtr) override {
		yaw = -yawSensitivity*(float)cursorX;
		pitch = -pitchSensitivity*(float)cursorY;
		if(pitch > 0.5f*M_PI) pitch = 0.5f*M_PI;
		else if(pitch < -0.5f*M_PI) pitch = -0.5f*M_PI;
		
		const float cy = cosf(yaw);
		const float sy = sinf(yaw);
		const float forward = (float)(ESDL::GetKeyDown(SDLK_w) - ESDL::GetKeyDown(SDLK_s));
		const float right = (float)(ESDL::GetKeyDown(SDLK_d) - ESDL::GetKeyDown(SDLK_a));
		position += speed*dT*(vec3){forward*cy + right*sy, forward*sy - right*cy, (float)(ESDL::GetKeyDown(SDLK_SPACE) - ESDL::GetKeyDown(SDLK_LSHIFT))};
		
		float a[4][4];
		M4x4_Scaling({2.0f, 2.0f, 2.0f}, perObjectDataPtr->model);
		M4x4_xRotation(0.5*M_PI, a);
		M4x4_PreMultiply(perObjectDataPtr->model, a);
		M4x4_zRotation(0.5*M_PI + yaw, a);
		M4x4_PreMultiply(perObjectDataPtr->model, a);
		M4x4_Translation({position.x, position.y, 0.0}, a);
		M4x4_PreMultiply(perObjectDataPtr->model, a);
		M4x4_Inverse(perObjectDataPtr->model, a);
		M4x4_Transpose(a, perObjectDataPtr->modelInvT);
	}
	
	void MouseMoved(SDL_Event event){
		cursorX += event.motion.xrel;
		cursorY += event.motion.yrel;
	}
	
	const float (&GetViewInverseMatrix())[4][4]{
		float a[4][4];
		M4x4_xRotation(0.5f*M_PI + pitch, viewInverseMatrix);
		M4x4_zRotation(-0.5f*M_PI + yaw, a);
		M4x4_PreMultiply(viewInverseMatrix, a);
		M4x4_Translation(position, a);
		M4x4_PreMultiply(viewInverseMatrix, a);
		M4x4_Inverse(viewInverseMatrix, viewInverseMatrix);
		return viewInverseMatrix;
	}
	
	const vec3 &GetCameraPosition(){ return position; }
	
private:
	float viewInverseMatrix[4][4];
	
	vec3 position;
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
	ChairInstanceManager(const int &_index) : Rendered::InstanceManager(_index, objDatas[(int)ObjData::chair]/*ReadProcessedOBJFile("ProcessedObjFiles/chair.bin", &GetTextureIdFromMtl)*/){}
	
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override {
		if(fragPcs) fragPcs->shininess = 1.0f;
		Rendered::InstanceManager::Render(pipeline, vertPcs, fragPcs, shadPcs);
	}
};
class ChairInstance : public Rendered::Instance {
public:
	ChairInstance(ChairInstanceManager *const &_manager) : Rendered::Instance(_manager) {
		position = {0.0f, 0.0f};
		angle = 0.0f;
	}
	
	void Update(const float &dT, Shared_Main::PerObject *const &perObjectDataPtr) override {
		float a[4][4];
		
		M4x4_xRotation(M_PI*0.5f, perObjectDataPtr->model);
		M4x4_zRotation(angle, a);
		M4x4_PreMultiply(perObjectDataPtr->model, a);
		M4x4_Translation({position.x, position.y, 0.0f}, a);
		M4x4_PreMultiply(perObjectDataPtr->model, a);
		
		M4x4_Inverse(perObjectDataPtr->model, a);
		M4x4_Transpose(a, perObjectDataPtr->modelInvT);
		
		memcpy(&savedInstanceData, perObjectDataPtr, sizeof(Shared_Main::PerObject));
		
		angle += dT*spinSpeed;
		const int fb = ESDL::GetKeyDown(SDLK_UP) - ESDL::GetKeyDown(SDLK_DOWN);
		const int rl = ESDL::GetKeyDown(SDLK_RIGHT) - ESDL::GetKeyDown(SDLK_LEFT);
		vec2 vel = speed*((float)fb*(vec2){-1.0f, 0.0f} + (float)rl*(vec2){0.0f, 1.0f});
		if(fb && rl) vel *= 0.70712f;
		position += vel*dT;
	}
	
	const Shared_Main::PerObject *GetInstanceData() const { return &savedInstanceData; }
	
private:
	vec2 position;
	float angle;
	float spinSpeed = 1.0f;
	float speed = 100.0f;
	
	Shared_Main::PerObject savedInstanceData;
};


class ChainSaw : public Rendered::Once {
public:
	ChainSaw(const int &_index, ChairInstance *const &_chair) : Rendered::Once(_index, objDatas[(int)ObjData::chainsaw]/*ReadProcessedOBJFile("ProcessedObjFiles/chainsaw.bin", &GetTextureIdFromMtl)*/), chair(_chair) {}
	
	void Update(const float &dT, Shared_Main::PerObject *const &perObjectDataPtr) override {
		memcpy(perObjectDataPtr, chair->GetInstanceData(), sizeof(Shared_Main::PerObject));
		float a[4][4];
		M4x4_Translation({-5.0f, 27.0f, 0.0f}, a);
		M4x4_PostMultiply(perObjectDataPtr->model, a);
		M4x4_zRotation(-0.5f*M_PI, a);
		M4x4_PostMultiply(perObjectDataPtr->model, a);
		M4x4_xRotation(0.5f*M_PI, a);
		M4x4_PostMultiply(perObjectDataPtr->model, a);
		M4x4_Scaling({5.0f, 5.0f, 5.0f}, a);
		M4x4_PostMultiply(perObjectDataPtr->model, a);
		M4x4_Inverse(perObjectDataPtr->model, a);
		M4x4_Transpose(a, perObjectDataPtr->modelInvT);
	}
	
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override {
		if(fragPcs) fragPcs->shininess = 100.0f;
		Rendered::Once::Render(pipeline, vertPcs, fragPcs, shadPcs);
	}
	
private:
	ChairInstance *chair;
};

class Plane : public Rendered::Once {
public:
	Plane(const int &_index) : Rendered::Once(_index, objDatas[(int)ObjData::plane]/*planeData*/) {
		
	}
	
	void Update(const float &dT, Shared_Main::PerObject *const &perObjectData) override {
		M4x4_Identity(perObjectData->model);
		M4x4_Identity(perObjectData->modelInvT);
	}
	
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override {
		if(fragPcs) fragPcs->shininess = 1.0f;
		Rendered::Once::Render(pipeline, vertPcs, fragPcs, shadPcs);
	}
};

Rendered::InstanceManager *renderedInstanced[Globals::MainInstanced::renderedN];
Rendered::Once *renderedOnce[Globals::MainOnce::renderedN];
Player *player;

void Update(const float &dT, Shared_Main::PushConstants_Vert &vertPcs, Shared_Main::PushConstants_Frag &fragPcs, Shared_Shadow::PushConstants_Vert &shadPcs){
	
	// UBOs
	Shared_Main::UBO_Global *const uboGlobalPointer = vulkanPtr->GetUniformBufferObjectPointer<Shared_Main::UBO_Global>((int)UBO::mainGlobal/*sharedMainGlobalUboIndex*/);
	static Shared_Main::PerObject *uboPerObjectPointers[Globals::MainOnce::renderedN];
	vulkanPtr->GetUniformBufferObjectPointers<Shared_Main::PerObject>((int)UBO::mainPerObject/*mainOncePerObjectUboIndex*/, uboPerObjectPointers);
	Shared_Shadow::UBO_Global *const uboShadowPointer = vulkanPtr->GetUniformBufferObjectPointer<Shared_Shadow::UBO_Global>((int)UBO::shadow/*sharedShadowGlobalUboIndex*/);
	Pipeline_Skybox::UBO_Global *const uboSkyboxPointer = vulkanPtr->GetUniformBufferObjectPointer<Pipeline_Skybox::UBO_Global>((int)UBO::skybox/*skyboxGlobalUboIndex*/);
	Pipeline_Hud::UBO *const uboHudPointer = vulkanPtr->GetUniformBufferObjectPointer<Pipeline_Hud::UBO>((int)UBO::hud/*hudUboIndex*/);
	
	// Updating
	for(int i=0; i<Globals::MainInstanced::renderedN; i++) renderedInstanced[i]->Update(dT);
	for(int i=0; i<Globals::MainOnce::renderedN; i++) renderedOnce[i]->Update(dT, uboPerObjectPointers[i]);
	
	// Setting main global UBO
	memcpy(uboGlobalPointer->viewInv, player->GetViewInverseMatrix(), 16*sizeof(float));
	M4x4_Perspective(M_PI*0.25f, (float)vulkanPtr->GetExtentWidth()/(float)vulkanPtr->GetExtentHeight(), Globals::cameraZNear, Globals::cameraZFar, uboGlobalPointer->proj);
	uboGlobalPointer->proj[1][1] *= -1.0f;
	memcpy(uboGlobalPointer->lightDir, &Globals::lightDirection, sizeof(vec3));
	const vec4 sunColour = {0.9882352941f, 0.8980392157f, 0.4392156863f, 1.0f};
	memcpy(uboGlobalPointer->lightColour, &sunColour, sizeof(vec4));
	memcpy(uboGlobalPointer->cameraPosition, &player->GetCameraPosition(), sizeof(vec3));
	
	// Setting shadow UBO and main lightMats
	UpdateCascades(uboGlobalPointer, uboShadowPointer);
	
	// Setting skybox UBO
	memcpy(uboSkyboxPointer->viewInv, uboGlobalPointer->viewInv, 16*sizeof(float));
	uboSkyboxPointer->viewInv[3][0] = uboSkyboxPointer->viewInv[3][1] = uboSkyboxPointer->viewInv[3][2] = 0.0f; // removing translational component
	memcpy(uboSkyboxPointer->proj, uboGlobalPointer->proj, 16*sizeof(float));
	uboSkyboxPointer->proj[2][2] = -1.0f; uboSkyboxPointer->proj[3][2] = 0.0f; // fix the depth at 1.0
	memcpy(uboSkyboxPointer->cameraPosition, &player->GetCameraPosition(), sizeof(vec3));
	
	// vertex shader push constants
	//currently placeholder
	
	// fragment shader push constants
	const vec4 white = {1.0f, 1.0f, 1.0f, 1.0f};
	memcpy(&fragPcs.colourMult, &white, sizeof(vec4));
	memcpy(&fragPcs.specular, &white, sizeof(vec4));
	fragPcs.shininess = 100.0f;
	fragPcs.specularFactor = 0.05f;
	
	// Setting HUD UBO
	*uboHudPointer = {(float32_t)vulkanPtr->GetExtentWidth(), (float32_t)vulkanPtr->GetExtentHeight(), 30.0f};
}

void RenderShadowMap(Shared_Shadow::PushConstants_Vert shadPcs, const int &cascadeLayer){
	shadPcs.cascadeLayer = cascadeLayer;
	
	vulkanPtr->GP((int)GraphicsPipeline::shadowInstanced).Bind();
	vulkanPtr->GP((int)GraphicsPipeline::shadowInstanced).BindDescriptorSets(0, Pipeline_ShadowInstanced::descriptorSetsN); // has no descriptor sets
	vulkanPtr->GP((int)GraphicsPipeline::shadowInstanced).CmdPushConstants<Shared_Shadow::PushConstants_Vert>(0, &shadPcs);
	for(int i=0; i<Globals::MainInstanced::renderedN; i++) renderedInstanced[i]->Render(GraphicsPipeline::shadowInstanced, nullptr, nullptr, &shadPcs);
	
	vulkanPtr->GP((int)GraphicsPipeline::shadowOnce).Bind();
	vulkanPtr->GP((int)GraphicsPipeline::shadowOnce).CmdPushConstants<Shared_Shadow::PushConstants_Vert>(0, &shadPcs);
	int indices[Pipeline_MainOnce::dynamicOffsetsN];
	for(int i=0; i<Globals::MainOnce::renderedN; i++){
		indices[0] = i;
		vulkanPtr->GP((int)GraphicsPipeline::shadowOnce).BindDescriptorSets(0, Pipeline_ShadowOnce::descriptorSetsN, Pipeline_MainOnce::dynamicOffsetsN, indices);
		renderedOnce[i]->Render(GraphicsPipeline::shadowOnce, nullptr, nullptr, &shadPcs);
	}
}

void RenderScene(Shared_Main::PushConstants_Vert vertPcs, Shared_Main::PushConstants_Frag fragPcs){
	vulkanPtr->GP((int)GraphicsPipeline::mainInstanced).Bind();
	vulkanPtr->GP((int)GraphicsPipeline::mainInstanced).BindDescriptorSets(0, Pipeline_MainInstanced::descriptorSetsN);
	for(int i=0; i<Globals::MainInstanced::renderedN; i++) renderedInstanced[i]->Render(GraphicsPipeline::mainInstanced, &vertPcs, &fragPcs, nullptr);
	
	vulkanPtr->GP((int)GraphicsPipeline::mainOnce).Bind();
	int indices[Pipeline_MainOnce::dynamicOffsetsN];
	for(int i=0; i<Globals::MainOnce::renderedN; i++){
		indices[0] = i;
		vulkanPtr->GP((int)GraphicsPipeline::mainOnce).BindDescriptorSets(0, Pipeline_MainOnce::descriptorSetsN, Pipeline_MainOnce::dynamicOffsetsN, indices);
		renderedOnce[i]->Render(GraphicsPipeline::mainOnce, &vertPcs, &fragPcs, nullptr);
	}
}

void RenderHUD(){
	vulkanPtr->GP((int)GraphicsPipeline::hud).Bind();
	vulkanPtr->GP((int)GraphicsPipeline::hud).BindDescriptorSets(0, Pipeline_Hud::descriptorSetsN);
	vulkanPtr->CmdBindVertexBuffer(0, Globals::HUD::vertexVBIndexOffset);
	vulkanPtr->CmdBindIndexBuffer(Globals::HUD::IBIndexOffset, VK_INDEX_TYPE_UINT32);
	vulkanPtr->CmdDrawIndexed(vulkanPtr->GetIndexBufferCount(Globals::HUD::IBIndexOffset));
}

int main(int argc, const char * argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	ESDL::InitEventHandler();
	
	IMG_Init(IMG_INIT_PNG);
	
	SDL_Window *window = SDL_CreateWindow("VulkanFirst", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	EVK::Devices devices = EVK::Devices(window);
	
	int skyboxImageIndex;
	{ // preparing skybox
		const char *cubemapFiles[6] = {
			"../Resources/textures/skybox_b.png", // correct
			"../Resources/textures/skybox_e.png", // correct
			"../Resources/textures/skybox_d.png", // correct
			"../Resources/textures/skybox_f.png", // correct
			"../Resources/textures/skybox_a.png", // correct
			"../Resources/textures/skybox_c.png" // correct
		};
		imageBlueprintPtrs[imagesBlueprinted] = new EVK::CubemapPNGImageBlueprint(cubemapFiles);
		skyboxImageIndex = imagesBlueprinted++;
	}
	
	int shadowImageIndex;
	{
		// For shadow mapping we only need a depth attachment
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.extent.width = SHADOWMAP_DIM;
		imageCI.extent.height = SHADOWMAP_DIM;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
		imageCI.format = DEPTH_FORMAT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL; // VK_IMAGE_TILING_LINEAR for row-major order if we want to access texels in the memory of the image
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageBlueprintPtrs[imagesBlueprinted] = new EVK::ManualImageBlueprint(imageCI, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
		shadowImageIndex = imagesBlueprinted++;
	}
	
	int finalColourImageIndex, finalDepthImageIndex;
	{
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.extent.width = 0; // resises with window
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.format = FINAL_FORMAT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL; // VK_IMAGE_TILING_LINEAR for row-major order if we want to access texels in the memory of the image
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
#ifdef MSAA
		imageCI.samples = devices.GetMSAASamples();
#else
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
#endif
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageBlueprintPtrs[imagesBlueprinted] = new EVK::ManualImageBlueprint(imageCI, VK_IMAGE_VIEW_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		finalColourImageIndex = imagesBlueprinted++;
		
		
		imageCI.format = devices.FindDepthFormat();
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageBlueprintPtrs[imagesBlueprinted] = new EVK::ManualImageBlueprint(imageCI, VK_IMAGE_VIEW_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
		finalDepthImageIndex = imagesBlueprinted++;
	}
	
	objDatas[(int)ObjData::player] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/MaleLow.bin", &GetTextureIdFromMtl);
	objDatas[(int)ObjData::chair] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/chair.bin", &GetTextureIdFromMtl);
	objDatas[(int)ObjData::chainsaw] = ReadProcessedOBJFile("../Resources/ProcessedObjFiles/chainsaw.bin", &GetTextureIdFromMtl);
	planeData.divisionData[0].texture = GetTextureIdFromMtl("concrete-917");
	objDatas[(int)ObjData::plane] = planeData;
	
	
	EVK::Interface vulkan = NewBuildPipelines(devices, imageBlueprintPtrs, shadowImageIndex, skyboxImageIndex, finalColourImageIndex, finalDepthImageIndex);
	vulkanPtr = &vulkan;
	
	vulkan.FillVertexBuffer(Globals::HUD::vertexVBIndexOffset, (void *)hudVertices, sizeof(hudVertices));
	vulkan.FillIndexBuffer(Globals::HUD::IBIndexOffset, (uint32_t *)hudIndices, hudIndicesN);
	
	vulkan.FillVertexBuffer(Globals::Skybox::vertexVBIndexOffset, (void *)skyboxVertices, sizeof(skyboxVertices));
	vulkan.FillIndexBuffer(Globals::Skybox::IBIndexOffset, (uint32_t *)skyboxIndices, skyboxIndicesN);
	
	vulkan.FillVertexBuffer(Globals::Finall::vertexVBIndexOffset, (void *)finalVertices, sizeof(finalVertices));
	vulkan.FillIndexBuffer(Globals::Finall::IBIndexOffset, (uint32_t *)finalIndices, finalIndicesN);
	
	ChairInstanceManager *chairManager;
	ChairInstance *chair;
	ChainSaw *chainSaw;
	Plane *plane;
	
	renderedInstanced[0] = chairManager = new ChairInstanceManager(0);
	chair = new ChairInstance(chairManager);
	
	renderedOnce[0] = chainSaw = new ChainSaw(0, chair);
	renderedOnce[1] = plane = new Plane(1);
	renderedOnce[2] = player = new Player(2, {100.0f, 0.0f});
	
	int time = SDL_GetTicks();
	
	while(!ESDL::HandleEvents()){
		
		const int newTime = SDL_GetTicks();
		const float dT = 0.001f*(float)(newTime - time);
		time = newTime;
		
		Shared_Main::PushConstants_Vert vertPcs;
		Shared_Main::PushConstants_Frag fragPcs;
		Shared_Shadow::PushConstants_Vert shadPcs;
		
		VkClearValue clearVals[2];
		clearVals[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
		clearVals[1].depthStencil = {1.0f, 0};
		
		Update(dT, vertPcs, fragPcs, shadPcs);
		
		if(vulkan.BeginFrame()){
			for(int i=0; i<SHADOW_MAP_CASCADE_COUNT; i++){
				vulkan.CmdBeginLayeredBufferedRenderPass(0, VK_SUBPASS_CONTENTS_INLINE, 1, &clearVals[1], i);
				//vulkan.CmdSetDepthBias(1.25f, 0.0f, 1.75f); // 1.25, 0.0, 1.75
				RenderShadowMap(shadPcs, i);
				vulkan.CmdEndRenderPass();
			}
			
			/*
			vulkan.BeginFinalRenderPass();
			
			vulkan.GP((int)GraphicsPipeline::skybox).Bind();
			vulkan.GP((int)GraphicsPipeline::skybox).BindDescriptorSets(0, Pipeline_Skybox::descriptorSetsN);
			vulkan.CmdBindVertexBuffer(0, Globals::Skybox::vertexVBIndexOffset);
			vulkan.CmdBindIndexBuffer(Globals::Skybox::IBIndexOffset, VK_INDEX_TYPE_UINT32);
			vulkan.CmdDrawIndexed(vulkan.GetIndexBufferCount(Globals::Skybox::IBIndexOffset));
			
			RenderScene(vertPcs, fragPcs);
			
			RenderHUD();
			 */
			
			vulkan.CmdBeginBufferedRenderPass(0, VK_SUBPASS_CONTENTS_INLINE, 2, clearVals);
			
			vulkan.GP((int)GraphicsPipeline::skybox).Bind();
			vulkan.GP((int)GraphicsPipeline::skybox).BindDescriptorSets(0, Pipeline_Skybox::descriptorSetsN);
			vulkan.CmdBindVertexBuffer(0, Globals::Skybox::vertexVBIndexOffset);
			vulkan.CmdBindIndexBuffer(Globals::Skybox::IBIndexOffset, VK_INDEX_TYPE_UINT32);
			vulkan.CmdDrawIndexed(vulkan.GetIndexBufferCount(Globals::Skybox::IBIndexOffset));
			
			RenderScene(vertPcs, fragPcs);
			
			RenderHUD();
			
			vulkan.CmdEndRenderPass();
			
			// compute commands recorded with a pipeline image memory barrier
			/*
			 - begin compute command buffer
			 - 'VkImageMemoryBarrier imageMemoryBarrier = {};
						 imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						 // We won't be changing the layout of the image
						 imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
						 imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
						 imageMemoryBarrier.image = textureComputeTarget.image;
						 imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
						 imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
						 imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						 imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						 imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						 vkCmdPipelineBarrier(
							 drawCmdBuffers[i],
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
							 VK_FLAGS_NONE,
							 0, nullptr,
							 0, nullptr,
							 1, &imageMemoryBarrier);'
			 - submit to compute queue
			 */
			
			
			// final render pass recorded with a pipeline buffer memory barrier:
			vulkan.BeginFinalRenderPass(); // begin with a pipeline buffer memory barrier
			
			vulkan.GP((int)GraphicsPipeline::finall).Bind();
			vulkan.GP((int)GraphicsPipeline::finall).BindDescriptorSets(0, 1);
			vulkan.CmdBindVertexBuffer(0, Globals::Finall::vertexVBIndexOffset);
			vulkan.CmdBindIndexBuffer(Globals::Finall::IBIndexOffset, VK_INDEX_TYPE_UINT32);
			vulkan.CmdDrawIndexed(vulkan.GetIndexBufferCount(Globals::Finall::IBIndexOffset));
			
			//
			vulkan.EndFinalRenderPassAndFrame();
		}
	}
	vkDeviceWaitIdle(vulkan.GetLogicalDevice());
	
	SDL_DestroyWindow(window);
	
	free(objDatas[(int)ObjData::player].vertices);
	free(objDatas[(int)ObjData::chair].vertices);
	free(objDatas[(int)ObjData::chainsaw].vertices);
	for(int i=0; i<Globals::MainInstanced::renderedN; i++) delete renderedInstanced[i];
	for(int i=0; i<Globals::MainOnce::renderedN; i++) delete renderedOnce[i];
	delete player;
	
	return 0;
}
