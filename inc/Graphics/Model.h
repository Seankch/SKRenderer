#ifndef MODEL_H
#define MODEL_H

/*****************************************************
	Includes
*****************************************************/
#include <vector>
#include <Graphics/GraphicsDefine.h>
#include <Graphics/Vulkan/VulkanTypes.h>

class Model
{
public:
	struct Mesh 
	{
		std::vector<glm::vec3> posVtxList;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvVertex;
		std::vector<glm::vec4> tangents;
		std::vector<unsigned> idxVertex;
		std::string meshName{};
		GPUMeshBuffers meshBuffer;
		glm::mat4 initialXFormMat;
		uint32_t indicesCount;
	};

	std::string modelName{};
	std::vector<Mesh> mMeshes;
	bool LoadModelFile(std::string const& _filePath);
};

#endif