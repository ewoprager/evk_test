#ifndef Pipelines_hpp
#define Pipelines_hpp

#include "Header.hpp"

//std::shared_ptr<EVK::Interface> NewBuildPipelines(const EVK::Devices &devices);
//void BuildVkInterfaceStructures(std::shared_ptr<EVK::Interface> interface, std::array<int, PNGS_N> pngsIndexArray);

std::array<std::shared_ptr<EVK::TextureSampler>, int(Sampler::_COUNT_)> BuildSamplers(std::shared_ptr<EVK::Devices> devices);

std::shared_ptr<EVK::LayeredBufferedRenderPass<SHADOW_MAP_CASCADE_COUNT>> BuildShadowMapRenderPass(std::shared_ptr<EVK::Devices> devices);

std::shared_ptr<EVK::BufferedRenderPass> BuildFinalRenderPass(std::shared_ptr<EVK::Devices> devices){

#endif /* Pipelines_hpp */
