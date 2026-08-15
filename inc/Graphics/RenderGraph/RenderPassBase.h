#ifndef RENDER_PASS_BASE_H
#define RENDER_PASS_BASE_H

/*****************************************************
    Includes
*****************************************************/
#include <vector>
#include <string>
#include <Graphics/Model.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Graphics/Material.h>
#include <Graphics/RenderContext.h>

class RenderPassBase
{
public:
    RenderPassBase(std::string const& _passName, std::vector<std::string> const& _inputs, std::vector<std::string> const& _outputs) : passName(_passName), inputs(_inputs), outputs(_outputs) {}

    // Execute render pass 
    virtual void Execute(RenderContext const& _renderContext) = 0;

    // Getters for passname, inputs, and outputs
    inline std::string GetPassName() 
    {
        return passName; 
    }

    inline std::vector<std::string> const& GetInputs() const 
    { 
        return inputs; 
    }

    inline std::vector<std::string> const& GetOutputs() const 
    { 
        return outputs; 
    }

private:
    std::vector<std::string> inputs;  // Input resources
    std::vector<std::string> outputs; // Output resources
    std::string passName;
};

#endif