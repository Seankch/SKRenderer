#ifndef SHADER_H
#define SHADER_H

#include <Graphics/GraphicsDefine.h>
#include <vector>
#include "vulkan/vulkan.h"

enum UniformType
{
	UT_FLOAT_VEC2,
	UT_FLOAT_VEC3,
	UT_FLOAT_VEC4,
	UT_FLOAT,
	UT_DOUBLE,
	UT_INT,
	UT_UNSIGNED_INT,
	UT_SAMPLER2D,
	UT_TEXTURE_INDEX
};

struct Uniform
{
	UniformType type{};
	std::string uniformName{};
};

class Shader
{
public:
	Shader();
	~Shader();

	// Shader start/end functions
	void Start();
	void End();

	// For adding uniforms during init
	void AddUniform(Uniform const& _uniform);

	// Get uniform variable list
	std::vector<Uniform> const& GetUniformList(void) const;

	// For base uniform variables
	static const std::vector<std::string> mBaseUniformVarList;
	std::string shaderName{};
private:
	// Stores loaded uniform variable information
	std::vector<Uniform> mUniformVarList;
};

#endif