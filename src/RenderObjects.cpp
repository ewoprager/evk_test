#include "RenderObjects.hpp"
#include "ReadProcessedObj.hpp"

extern std::shared_ptr<EVK::Interface> vulkan;

namespace Rendered {

Once::Once(int _index, const ObjectData &_objData) : index(_index), objData(_objData) {
	vulkan->FillVertexBuffer(Globals::MainOnce::vertexVBIndexOffset + _index, (void *)_objData.vertices, _objData.vertices_n*sizeof(Pipeline_MainOnce::Attributes::Vertex));
}
void Once::Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs){
	
	if(vertPcs) vulkan->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Vert>((int)Shared_Main::PushConstantRange::vert, vertPcs);
	
	vulkan->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::vertex, Globals::MainOnce::vertexVBIndexOffset + index);
	for(int i=0; i<objData.divisionsN; i++){
		if(fragPcs){
			fragPcs->textureID = (int32_t)objData.divisionData[i].texture;
			vulkan->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Frag>((int)Shared_Main::PushConstantRange::frag, fragPcs);
		}
		vulkan->CmdDraw((uint32_t)objData.divisionData[i].count, 1, objData.divisionData[i].start);
	}
}

InstanceManager::InstanceManager(int _index, const ObjectData &_objData) : index(_index), objData(_objData) {
	vulkan->FillVertexBuffer(Globals::MainInstanced::vertexVBIndexOffset + _index, (void *)_objData.vertices, _objData.vertices_n*sizeof(Pipeline_MainInstanced::Attributes::Vertex));
}
void InstanceManager::Update(float dT){
	for(int i=0; i<instanceCount; i++) instances[i]->Update(dT, &instanceData[i]);
	vulkan->FillVertexBuffer(Globals::MainInstanced::vertexVBIndexOffset + Globals::MainInstanced::renderedN + index, (void *)instanceData, instanceCount*sizeof(Shared_Main::PerObject));
}
void InstanceManager::Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs){
	
	if(vertPcs) vulkan->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Vert>((int)Shared_Main::PushConstantRange::vert, vertPcs);
	
	vulkan->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::vertex, Globals::MainInstanced::vertexVBIndexOffset + index);
	vulkan->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::instance, Globals::MainInstanced::vertexVBIndexOffset + Globals::MainInstanced::renderedN + index);
	for(int i=0; i<objData.divisionsN; i++){
		if(fragPcs){
			fragPcs->textureID = (int32_t)objData.divisionData[i].texture;
			vulkan->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Frag>((int)Shared_Main::PushConstantRange::frag, fragPcs);
		}
		vulkan->CmdDraw((uint32_t)objData.divisionData[i].count, instanceCount, objData.divisionData[i].start);
	}
}

Instance::Instance(InstanceManager *_manager) : manager(_manager) {
	_manager->AddInstance(this);
}
Instance::~Instance(){
	manager->RemoveInstance(this);
}

}

