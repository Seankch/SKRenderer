#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

/*****************************************************
	Includes
*****************************************************/
#include "Managers/RendererManager.h"
#include "Managers/EntityManager.h"
#include "Managers/ResourceManager.h"

class RenderContext
{
public:
    // Constructor
    RenderContext(size_t _fboIdx, glm::mat4 const& _viewMatrix, glm::mat4 const& _projectionMatrix, RendererManager& _rendererMgr, EntityManager& _entityMgr, ResourceManager& _resourceMgr)
        : fboIdx(_fboIdx), V(_viewMatrix), P(_projectionMatrix), rendererMgr(_rendererMgr), entityMgr(_entityMgr), resourceMgr(_resourceMgr) {}
    
    // Render context variables
    size_t fboIdx;
    glm::mat4 V; // View
    glm::mat4 P; // Projection
    RendererManager& rendererMgr;
    EntityManager& entityMgr;
    ResourceManager& resourceMgr;
};

#endif