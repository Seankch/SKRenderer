#ifndef FINAL_PASS_H
#define FINAL_PASS_H

/*****************************************************
    Includes
*****************************************************/
#include "Graphics/RenderGraph/RenderPassBase.h"
#include <vector>
#include <string>

class FinalPass : public RenderPassBase
{
public:
    // Inherit constructor from parent
    using RenderPassBase::RenderPassBase;

    // Execute render pass 
    void Execute(RenderContext const& _renderContext);
};

#endif