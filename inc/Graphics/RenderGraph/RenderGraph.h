#ifndef RENDER_GRAPH_H
#define RENDER_GRAPH_H

/*****************************************************
    Includes
*****************************************************/
#include <vector>
#include <string>
#include <unordered_map>
#include "Graphics/RenderGraph/RenderPassBase.h"

class RenderGraph
{
public:
    // Add render pass (doesn't create any resources yet)
    void AddPass(RenderPassBase* _renderPass);

    // Compile the render graph, create resources and arrange graph
    void Compile(void);

    // Execute the render graph
    void Execute(RenderContext const& _renderContext);

    // Destroy passes
    void Destroy(void);
private:
    std::unordered_map<std::string, RenderPassBase*> renderPasses;
    std::vector<RenderPassBase*> sortedRenderPasses;
};

#endif