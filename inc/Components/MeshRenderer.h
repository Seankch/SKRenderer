#ifndef MESH_RENDERER_H
#define MESH_RENDERER_H

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Material.h>
#include <Graphics/Model.h>
#include <unordered_map>

// Class to encapsulate mesh rendering data
class MeshRenderer
{
public:
    MeshRenderer();
    ~MeshRenderer();

    // Getters
    std::string const& GetModel(void) const;
    bool IsRendering(void) const;
    std::vector<std::string>& GetMaterialList(void);
    std::vector<Material>& GetMatInstanceList(void);
    Material* GetMatInstance(int _index);

    // Setters
    void SetModel(Model const& _model);
    void SetMaterial(int _index, std::string const& _matName);
    void SetIsRendering(bool _isRendering);

    // Adding/clearing materials
    void AddMatInstance(Material const& _mat);
    void ClearMaterialList(void);
private:
    // Variables for mesh renderer
    std::string mModelName;
    bool mIsRendering{};

    // Variables for applying material on each submesh
    std::vector<std::string> mMatList;

    // For holding material instances
    std::vector<Material> mMatInstanceList;
};

#endif