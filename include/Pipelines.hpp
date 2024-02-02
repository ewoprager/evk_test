#ifndef Pipelines_hpp
#define Pipelines_hpp

#include "Header.hpp"

std::shared_ptr<EVK::Interface> NewBuildPipelines(const EVK::Devices &devices);
void BuildVkInterfaceStructures(std::shared_ptr<EVK::Interface> interface, std::array<int, PNGS_N> pngsIndexArray);

#endif /* Pipelines_hpp */
