#ifndef RenderObjects_hpp
#define RenderObjects_hpp

#include "Header.hpp"

namespace Rendered {

class Parent;
class Once;
class InstanceManager;
class Instance;

struct Info {
	uint32_t n;
	float shininess;
		
	struct Draw {
		int32_t textureId;
		uint32_t vertexCount;
		uint32_t instanceCount = 1;
		uint32_t firstVertex = 0;
		uint32_t firstInstance = 0;
	};
	std::function<Draw(uint32_t)> drawFunction;
};

class Once {
public:
	Once(std::shared_ptr<EVK::Devices> _devices, const ObjectData &_objData);
	~Once() = default;
	
	virtual void Update(float dT, PerObject *perObjectDataPtr) {}
	
	virtual Info Render(VkCommandBuffer commandBuffer);
	
private:
	std::shared_ptr<EVK::Devices> devices;
	std::shared_ptr<EVK::VertexBufferObject> vbo;
	ObjectData objData;
};

class Instance {
public:
	Instance(InstanceManager *_manager);
	~Instance();
	
	virtual void Update(float dT, PerObject *perObjectDataPtr) {}
	
private:
	InstanceManager *manager;
};

class InstanceManager {
public:
	InstanceManager(std::shared_ptr<EVK::Devices> _devices, const ObjectData &_objData);
	~InstanceManager() = default;
	
	void Update(float dT);
	
	virtual Info Render(VkCommandBuffer commandBuffer);
	
	void AddInstance(Instance *ptr){
		instances[instanceCount++] = ptr;
	}
	
	void RemoveInstance(Instance *ptr){
		for(int i=0; i<instanceCount; i++){
			if(instances[i] == ptr){
				memcpy(&instances[i], &instances[i + 1], instanceCount - i - 1);
				instanceCount--;
				return;
			}
		}
		std::cout << "Warning: Tried to remove an devices.instance that wasn't added.\n";
	}
	
private:
	std::shared_ptr<EVK::Devices> devices;
	std::shared_ptr<EVK::VertexBufferObject> vboVertex;
	std::shared_ptr<EVK::VertexBufferObject> vboInstance;
	ObjectData objData;
	
	PerObject instanceData[MAX_INSTANCES];
	Instance *instances[MAX_INSTANCES];
	int instanceCount = 0;
};

} // namespace Rendered

#endif /* RenderObjects_hpp */
