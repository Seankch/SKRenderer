#include "Graphics/RenderGraph/RenderGraph.h"

void RenderGraph::AddPass(RenderPassBase* _renderPass)
{
    // Add to render passes map
    renderPasses.emplace(_renderPass->GetPassName(), _renderPass);
}

void RenderGraph::Compile(void)
{
    // Sort render passes order based on inputs/outputs

}

void RenderGraph::Execute(RenderContext const& _renderContext)
{
    // Execute renderpasses
    for (int i = 0; i < sortedRenderPasses.size(); ++i)
    {
        sortedRenderPasses[i]->Execute(_renderContext);
    }
}

void RenderGraph::Destroy(void)
{
    // Destroy renderpasses
    for (int i = 0; i < sortedRenderPasses.size(); ++i)
    {
        delete sortedRenderPasses[i];
    }
    sortedRenderPasses.clear();
}
