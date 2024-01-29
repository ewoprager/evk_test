#ifndef Pipelines_hpp
#define Pipelines_hpp

#include "Header.hpp"

std::shared_ptr<EVK::Interface> NewBuildPipelines(const EVK::Devices &devices, const std::vector<std::shared_ptr<EVK::IImageBlueprint>> &imageBlueprintPtrs, int shadowMapImageIndex, int skyboxImageIndex, int finalColourImageIndex, int finalDepthImageIndex);

#endif /* Pipelines_hpp */
