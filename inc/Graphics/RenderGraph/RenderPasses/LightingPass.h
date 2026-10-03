#ifndef LIGHTING_PASS_H
#define LIGHTING_PASS_H

/*****************************************************
    Includes
*****************************************************/
#include "Graphics/RenderGraph/RenderPassBase.h"
#include <vector>
#include <string>

class LightingPass : public RenderPassBase
{
public:
    // Inherit constructor from parent
    using RenderPassBase::RenderPassBase;

    // Execute render pass 
    void Execute(RenderContext const& _renderContext);
};

#endif