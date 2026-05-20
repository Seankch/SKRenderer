/*****************************************************
    Includes
*****************************************************/
// For parsing .glb json chunk
#pragma warning(push, 0)
#include <document.h>
#include <error/en.h>
#pragma warning(pop)

// Other includes
#include <Graphics/Model.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <Managers/RendererManager.h>

std::vector<char> ReadBinaryFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read file: " + filePath);
    }

    return buffer;
}

bool Model::LoadModelFile(std::string const& _filePath) 
{
    // Read binary file
    std::vector<char> fileData = ReadBinaryFile(_filePath);

    // Parse the GLB header
    uint32_t magic = *reinterpret_cast<uint32_t*>(&fileData[0]);
    if (magic != 0x46546C67) // "glTF"
        throw std::runtime_error("Invalid GLB file");

    // Parse the JSON chunk header
    uint32_t jsonChunkLength = *reinterpret_cast<uint32_t*>(&fileData[12]);
    uint32_t jsonChunkType = *reinterpret_cast<uint32_t*>(&fileData[16]);

    // Check 'JSON' chunk type
    if (jsonChunkType != 0x4E4F534A) 
        throw std::runtime_error("Invalid JSON chunk in GLB file");

    // Parse JSON chunk
    std::string jsonChunk(fileData.begin() + 20, fileData.begin() + 20 + jsonChunkLength);
    rapidjson::Document doc;
    if (doc.Parse(jsonChunk.c_str()).HasParseError())
        throw std::runtime_error("Failed to parse JSON chunk: " + std::string(rapidjson::GetParseError_En(doc.GetParseError())));

    // Accessors and bufferViews
    auto& accessors = doc["accessors"];
    auto& bufferViews = doc["bufferViews"];

    // Load binary buffer
    uint8_t* binaryData = reinterpret_cast<uint8_t*>(&fileData[20 + jsonChunkLength + 8]); // Skip JSON chunk and its length
    size_t binaryDataSize = fileData.size() - (20 + jsonChunkLength + 8);

    // Iterate thru meshes to parse positions, texcoords, etc
    rapidjson::Value const& meshes = doc["meshes"];
    for (rapidjson::SizeType meshID = 0; meshID < meshes.Size(); ++meshID)
    {
        // Add new mesh to model
        mMeshes.emplace_back(Mesh{});
        Mesh& currMesh = mMeshes.back();
        currMesh.initialXFormMat = glm::mat4(1.f);

        // Load primitive data
        auto& primitives = meshes[meshID]["primitives"];
        currMesh.meshName = meshes[meshID]["name"].GetString();
        for (auto& primitive : primitives.GetArray())
        {
            // Positions
            if (primitive["attributes"].HasMember("POSITION"))
            {
                auto& accessor = accessors[primitive["attributes"]["POSITION"].GetUint()];
                auto& bufferView = bufferViews[accessor["bufferView"].GetUint()];
                uint32_t count = accessor["count"].GetUint();
                uint32_t accessorOffset = accessor.HasMember("byteOffset") ? accessor["byteOffset"].GetUint() : 0;
                uint32_t bufferOffset = bufferView.HasMember("byteOffset") ? bufferView["byteOffset"].GetUint() : 0;
                uint32_t stride = bufferView.HasMember("byteStride") ? bufferView["byteStride"].GetUint() : sizeof(glm::vec3);

                const uint8_t* data = binaryData + bufferOffset + accessorOffset;
                currMesh.posVtxList.resize(count);
                for (uint32_t i = 0; i < count; i++)
                {
                    std::memcpy(&currMesh.posVtxList[i], data + i * stride, sizeof(glm::vec3));
                }
            }
            
            // Normals
            if (primitive["attributes"].HasMember("NORMAL"))
            {
                auto& accessor = accessors[primitive["attributes"]["NORMAL"].GetUint()];
                auto& bufferView = bufferViews[accessor["bufferView"].GetUint()];
                uint32_t count = accessor["count"].GetUint();
                uint32_t accessorOffset = accessor.HasMember("byteOffset") ? accessor["byteOffset"].GetUint() : 0;
                uint32_t bufferOffset = bufferView.HasMember("byteOffset") ? bufferView["byteOffset"].GetUint() : 0;
                uint32_t stride = bufferView.HasMember("byteStride") ? bufferView["byteStride"].GetUint() : sizeof(glm::vec3);

                const uint8_t* data = binaryData + bufferOffset + accessorOffset;
                currMesh.normals.resize(count);
                for (uint32_t i = 0; i < count; i++)
                {
                    std::memcpy(&currMesh.normals[i], data + i * stride, sizeof(glm::vec3));
                }
            }

            // UVs
            if (primitive["attributes"].HasMember("TEXCOORD_0"))
            {
                auto& accessor = accessors[primitive["attributes"]["TEXCOORD_0"].GetUint()];
                auto& bufferView = bufferViews[accessor["bufferView"].GetUint()];
                uint32_t count = accessor["count"].GetUint();
                uint32_t accessorOffset = accessor.HasMember("byteOffset") ? accessor["byteOffset"].GetUint() : 0;
                uint32_t bufferOffset = bufferView.HasMember("byteOffset") ? bufferView["byteOffset"].GetUint() : 0;
                uint32_t stride = bufferView.HasMember("byteStride") ? bufferView["byteStride"].GetUint() : sizeof(glm::vec2);

                const uint8_t* data = binaryData + bufferOffset + accessorOffset;
                currMesh.uvVertex.resize(count);
                for (uint32_t i = 0; i < count; i++)
                {
                    std::memcpy(&currMesh.uvVertex[i], data + i * stride, sizeof(glm::vec2));
                }
            }

            // Indices
            if (primitive.HasMember("indices"))
            {
                auto& accessor = accessors[primitive["indices"].GetUint()];
                auto& bufferView = bufferViews[accessor["bufferView"].GetUint()];
                uint32_t accessorOffset = accessor.HasMember("byteOffset") ? accessor["byteOffset"].GetUint() : 0;
                uint32_t bufferOffset = bufferView.HasMember("byteOffset") ? bufferView["byteOffset"].GetUint() : 0;

                const uint8_t* data = binaryData + bufferOffset + accessorOffset;
                uint32_t count = accessor["count"].GetUint();
                uint32_t componentType = accessor["componentType"].GetUint();
                currMesh.idxVertex.resize(count);
                for (uint32_t i = 0; i < count; i++)
                {
                    uint32_t index = 0;

                    if (componentType == 5125) // UINT
                    {
                        std::memcpy(&index, data + i * 4, 4);
                    }
                    else if (componentType == 5123) // USHORT
                    {
                        uint16_t temp;
                        std::memcpy(&temp, data + i * 2, 2);
                        index = temp;
                    }
                    else if (componentType == 5121) // UBYTE
                    {
                        uint8_t temp;
                        std::memcpy(&temp, data + i, 1);
                        index = temp;
                    }

                    currMesh.idxVertex[i] = index;
                }

                currMesh.indicesCount = count;
            }
        }

        // Generate tangents
        size_t faceCount = currMesh.idxVertex.size() / 3;
        std::vector<glm::vec3> tan1Accum(currMesh.posVtxList.size());
        std::vector<glm::vec3> tan2Accum(currMesh.posVtxList.size());
        currMesh.tangents.resize(currMesh.posVtxList.size());
        for (size_t i = 0; i < faceCount; ++i)
        {
            // Get indices of the triangle vertices
            unsigned idx0 = currMesh.idxVertex[i * 3];
            unsigned idx1 = currMesh.idxVertex[i * 3 + 1];
            unsigned idx2 = currMesh.idxVertex[i * 3 + 2];

            glm::vec3& p1 = currMesh.posVtxList[idx0];
            glm::vec3& p2 = currMesh.posVtxList[idx1];
            glm::vec3& p3 = currMesh.posVtxList[idx2];

            glm::vec2& tc1 = currMesh.uvVertex[idx0];
            glm::vec2& tc2 = currMesh.uvVertex[idx1];
            glm::vec2& tc3 = currMesh.uvVertex[idx2];

            glm::vec3 q1 = p2 - p1;
            glm::vec3 q2 = p3 - p1;
            float s1 = tc2.x - tc1.x, s2 = tc3.x - tc1.x;
            float t1 = tc2.y - tc1.y, t2 = tc3.y - tc1.y;
            float r = 1.0f / (s1 * t2 - s2 * t1);
            glm::vec3 tan1((t2 * q1.x - t1 * q2.x) * r,
                (t2 * q1.y - t1 * q2.y) * r,
                (t2 * q1.z - t1 * q2.z) * r);
            glm::vec3 tan2((s1 * q2.x - s2 * q1.x) * r,
                (s1 * q2.y - s2 * q1.y) * r,
                (s1 * q2.z - s2 * q1.z) * r);
            tan1Accum[idx0] += tan1;
            tan1Accum[idx1] += tan1;
            tan1Accum[idx2] += tan1;
            tan2Accum[idx0] += tan2;
            tan2Accum[idx1] += tan2;
            tan2Accum[idx2] += tan2;
        }

        // Normalize tangents and orthogonalize using Gram-Schmidt
        for (size_t i = 0; i < currMesh.posVtxList.size(); ++i)
        {
            const glm::vec3& n = currMesh.normals[i];
            glm::vec3& t1 = tan1Accum[i];
            glm::vec3& t2 = tan2Accum[i];

            // Gram-Schmidt orthogonalize
            currMesh.tangents[i] = glm::vec4(glm::normalize(t1 - (glm::dot(n, t1) * n)), 0.0f);
            // Store handedness in w
            currMesh.tangents[i].w = (glm::dot(glm::cross(n, t1), t2) < 0.0f) ? -1.0f : 1.0f;
        }
    }

    // Parse transform matrices
    auto& nodes = doc["nodes"];
    for (rapidjson::SizeType nodeID = 0; nodeID < nodes.Size(); ++nodeID) 
    {
        for (int currIdx = 0; currIdx < mMeshes.size(); ++currIdx)
        {
            if (nodes[nodeID].HasMember("matrix"))
            {
                // Check if it's loading matrix for the correct mesh
                std::string currMeshName = nodes[nodeID]["name"].GetString();
                if (mMeshes[currIdx].meshName != currMeshName)
                    continue;

                // Load initial transform mat for mesh
                auto const& matrixArray = nodes[nodeID]["matrix"].GetArray();
                for (rapidjson::SizeType i = 0; i < matrixArray.Size(); ++i)
                {
                    // Store matrix array into glm::mat4
                    mMeshes[currIdx].initialXFormMat[i / 4][i % 4] = matrixArray[i].GetFloat();
                }
            }
        }
    }

    // Return true once done
    return true;
}
