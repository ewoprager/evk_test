#ifndef CascadedShadowMap_hpp
#define CascadedShadowMap_hpp

#include "Header.hpp"

// requires the camera projection and viewInverse matrices to be already set in the Main Global UBO
void UpdateCascades(Shared_Main::UBO_Global *mainUboGlobal, Shared_Shadow::UBO_Global *shadowUboGlobal);

#endif /* CascadedShadowMap_hpp */
