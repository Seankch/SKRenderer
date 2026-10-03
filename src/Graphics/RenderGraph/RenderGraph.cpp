#include "Graphics/RenderGraph/RenderGraph.h"

void RenderGraph::AddPass(RenderPassBase* _renderPass)
{
    // Add to render passes map
    renderPasses.push_back(_renderPass);

    // Add to adjacency list
    adjacencyList.push_back(std::vector<size_t>());
    for (int i = 0; i < _renderPass->)
}

void RenderGraph::Compile(void)
{
    // Sort render passes order based on inputs/outputs
    TopologicalSort();

    // Create resources for each render pass

}

void RenderGraph::Execute(RenderContext const& _renderContext)
{
    // Execute renderpasses
    for (int i = 0; i < renderPasses.size(); ++i)
    {
        renderPasses[i]->Execute(_renderContext);
    }
}

void RenderGraph::Destroy(void)
{
    // Destroy renderpasses
    for (int i = 0; i < renderPasses.size(); ++i)
    {
        delete renderPasses[i];
    }
    renderPasses.clear();
}

void RenderGraph::TopologicalSort(void)
{
    // Init visited list
    std::vector<bool> visited(renderPasses.size(), false);

    // Create temp list
    std::vector<RenderPassBase*> tempList{ renderPasses };
    renderPasses.clear();

    // Perform DFS
    for (int i = 0; i < tempList.size(); ++i)
    {
        if (!visited[i])
        {
            DFS(i, visited, tempList);
        }
    }

    // Reverse sorted list
    reverse(renderPasses.begin(), renderPasses.end());
}

void RenderGraph::DFS(int _index, std::vector<bool>& _visited, std::vector<RenderPassBase*> const& _passList)
{
    // Set visited to true
    _visited[_index] = true;

    // Visit all neighbors for this node
    for (size_t neighborIndex : adjacencyList[_index])
    {
        if (!_visited[neighborIndex])
        {
            DFS(neighborIndex, _visited, _passList);
        }
    }

    // Add to render pass
    renderPasses.push_back(_passList[_index]);
}
