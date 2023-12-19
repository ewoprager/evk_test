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
	virtual void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs){}
};

class Once : public Parent {
public:
	Once(const int &_index, const ObjectData &_objData);
	~Once(){}
	
	virtual void Update(const float &dT, Shared_Main::PerObject *const &perObjectDataPtr){}
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override;
	
private:
	int index;
	ObjectData objData;
};

class Instance {
public:
	Instance(InstanceManager *const &_manager);
	~Instance();
	
	virtual void Update(const float &dT, Shared_Main::PerObject *const &perObjectDataPtr){}
	
private:
	InstanceManager *manager;
};

class InstanceManager : public Parent {
public:
	InstanceManager(const int &_index, const ObjectData &_objData);
	~InstanceManager(){}
	
	void Update(const float &dT);
	void Render(const GraphicsPipeline &pipeline, Shared_Main::PushConstants_Vert *vertPcs, Shared_Main::PushConstants_Frag *fragPcs, Shared_Shadow::PushConstants_Vert *shadPcs) override;
	
	void AddInstance(Instance *const &ptr){
		instances[instanceCount++] = ptr;
	}
	void RemoveInstance(Instance *const &ptr){
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
	
	Shared_Main::PerObject instanceData[MAX_INSTANCES];
	Instance *instances[MAX_INSTANCES];
	int instanceCount = 0;
};

}

#endif /* RenderObjects_hpp */
