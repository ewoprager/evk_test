#include "PipelineMain.hpp"
#include "ReadProcessedObj.hpp"

#include "RenderObjects.hpp"

namespace Rendered {

Once::Once(std::shared_ptr<EVK::Devices> _devices, const ObjectData &_objData) : devices(_devices), objData(_objData) {
	vbo = std::make_shared<EVK::VertexBufferObject>(devices);
	vbo->Fill((void *)_objData.vertices, _objData.vertices_n * sizeof(PipelineMain::Vertex));
}
Info Once::Render(VkCommandBuffer commandBuffer) {
	vbo->CmdBind(commandBuffer, uint32_t(VertexBufferBinding::vertex));
	return {
		.n = objData.divisionsN,
		.shininess = 1.0f,
		.drawFunction = [&](uint32_t index) -> Info::Draw {
			return {
				.textureId = int(objData.divisionData[index].texture),
				.vertexCount = uint32_t(objData.divisionData[index].count),
				.instanceCount = 1,
				.firstVertex = uint32_t(objData.divisionData[index].start)
			};
		}
	};
	
//	if(vertPcs) interface->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Vert>((int)Shared_Main::PushConstantRange::vert, vertPcs);
//	
//	interface->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::vertex, Globals::MainOnce::vertexVBIndexOffset + index);
//	for(int i=0; i<objData.divisionsN; ++i){
//		if(fragPcs){
//			fragPcs->textureID = (int32_t)objData.divisionData[i].texture;
//			interface->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Frag>((int)Shared_Main::PushConstantRange::frag, fragPcs);
//		}
//		interface->CmdDraw((uint32_t)objData.divisionData[i].count, 1, objData.divisionData[i].start);
//	}
}

InstanceManager::InstanceManager(std::shared_ptr<EVK::Devices> _devices, const ObjectData &_objData) : devices(_devices), objData(_objData) {
	vboVertex = std::make_shared<EVK::VertexBufferObject>(devices);
	vboInstance = std::make_shared<EVK::VertexBufferObject>(devices);
	vboVertex->Fill((void *)_objData.vertices, _objData.vertices_n * sizeof(PipelineMain::Vertex));
}
void InstanceManager::Update(float dT){
	for(int i=0; i<instanceCount; ++i){
		instances[i]->Update(dT, &instanceData[i]);
	}
	vboInstance->Fill((void *)instanceData, instanceCount * sizeof(PerObject));
}
Info InstanceManager::Render(VkCommandBuffer commandBuffer){
	vboVertex->CmdBind(commandBuffer, uint32_t(VertexBufferBinding::vertex));
	vboInstance->CmdBind(commandBuffer, uint32_t(VertexBufferBinding::instance));
	return {
		.n = objData.divisionsN,
		.shininess = 1.0f,
		.drawFunction = [&](uint32_t index) -> Info::Draw {
			return {
				.textureId = int(objData.divisionData[index].texture),
				.vertexCount = uint32_t(objData.divisionData[index].count),
				.instanceCount = uint32_t(instanceCount),
				.firstVertex = uint32_t(objData.divisionData[index].start)
			};
		}
	};
	
//	if(vertPcs){ interface->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Vert>((int)Shared_Main::PushConstantRange::vert, vertPcs);
//	}
//	
//	interface->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::vertex, Globals::MainInstanced::vertexVBIndexOffset + index);
//	interface->CmdBindVertexBuffer((uint32_t)VertexBufferBinding::instance, Globals::MainInstanced::vertexVBIndexOffset + Globals::MainInstanced::renderedN + index);
//	for(int i=0; i<objData.divisionsN; i++){
//		if(fragPcs){
//			fragPcs->textureID = (int32_t)objData.divisionData[i].texture;
//			interface->GP((int)pipeline).CmdPushConstants<Shared_Main::PushConstants_Frag>((int)Shared_Main::PushConstantRange::frag, fragPcs);
//		}
//		interface->CmdDraw((uint32_t)objData.divisionData[i].count, instanceCount, objData.divisionData[i].start);
//	}
}

Instance::Instance(InstanceManager *_manager) : manager(_manager) {
	_manager->AddInstance(this);
}
Instance::~Instance(){
	manager->RemoveInstance(this);
}

} // namespace Rendered

