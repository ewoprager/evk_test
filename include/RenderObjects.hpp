#ifndef RenderObjects_hpp
#define RenderObjects_hpp

#include "Header.hpp"

namespace Rendered {

class Parent;
class Once;
class InstanceManager;
class Instance;


class Parent {
public:
	Parent(std::shared_ptr<EVK::Interface> _interface) : interface(_interface) {}
	
	virtual void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) = 0;
	
protected:
	std::shared_ptr<EVK::Interface> interface;
};

class Once : public Parent {
public:
	Once(int _index, const ObjectData &_objData);
	~Once(){}
	
	virtual void Update(float dT, Shared_Main::PerObject *perObjectDataPtr){}
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override;
	
private:
	std::shared_ptr<EVK::VertexBufferObject> vbo;
	ObjectData objData;
};

class Instance {
public:
	Instance(InstanceManager *_manager);
	~Instance();
	
	virtual void Update(float dT, Shared_Main::PerObject *perObjectDataPtr){}
	
private:
	InstanceManager *manager;
};

class InstanceManager : public Parent {
public:
	InstanceManager(int _index, const ObjectData &_objData);
	~InstanceManager(){}
	
	void Update(float dT);
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override;
	
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
	int index;
	ObjectData objData;
	
	PerObject instanceData[MAX_INSTANCES];
	Instance *instances[MAX_INSTANCES];
	int instanceCount = 0;
};

}

#endif /* RenderObjects_hpp */
