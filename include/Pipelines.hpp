#ifndef Pipelines_hpp
#define Pipelines_hpp

#include "Header.hpp"

EVK::Interface NewBuildPipelines(const EVK::Devices &devices, EVK::ImageBlueprint **const &imageBlueprintPtrs, int shadowMapImageIndex, int skyboxImageIndex, int finalColourImageIndex, int finalDepthImageIndex);

#endif /* Pipelines_hpp */
