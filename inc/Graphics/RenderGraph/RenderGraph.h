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
    // Misc functions
    void TopologicalSort(void);
    void DFS(int _index, std::vector<bool>& _visited, std::vector<RenderPassBase*> const& _passList);

    // Render passes
    std::vector<RenderPassBase*> renderPasses;
    std::vector<std::vector<size_t>> adjacencyList; // For topo sort
};

#endif