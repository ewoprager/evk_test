#ifndef CascadedShadowMap_hpp
#define CascadedShadowMap_hpp

#include "PipelineShadow.hpp"
#include "PipelineMain.hpp"

// requires the camera projection and viewInverse matrices to be already set in the Main Global UBO
void UpdateCascades(PipelineMain::UBO_Global *mainUboGlobal, PipelineShadow::UBO_Global *shadowUboGlobal);

#endif /* CascadedShadowMap_hpp */
