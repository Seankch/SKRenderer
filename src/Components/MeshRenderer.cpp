#include <Components/MeshRenderer.h>
#include "Managers/GraphicsSystem.h"
#include "DIContainer/DIContainer.h"

MeshRenderer::MeshRenderer()
{
	mModelName = {};
	mIsRendering = true;
}

MeshRenderer::~MeshRenderer()
{
}

std::string const& MeshRenderer::GetModel(void) const
{
	return mModelName;
}

std::vector<std::string>& MeshRenderer::GetMaterialList(void)
{
	return mMatList;
}

std::vector<Material>& MeshRenderer::GetMatInstanceList(void)
{
    return mMatInstanceList;
}

bool MeshRenderer::IsRendering(void) const
{
	return mIsRendering;
}

Material* MeshRenderer::GetMatInstance(int _index)
{
	if (_index >= mMatInstanceList.size())
		return nullptr;

	return &mMatInstanceList[_index];
}

void MeshRenderer::SetModel(Model const& _model)
{
	// If model name is same, no need to change
	if (mModelName == _model.modelName)
		return;

	// Set model name
	mModelName = _model.modelName;
	
	// Resize material list size based on submesh count
	mMatList.clear();
	mMatList.resize(_model.mMeshes.size());
}

void MeshRenderer::SetMaterial(int _index, std::string const& _matName)
{
	if (_index >= mMatList.size())
	{
		return;
	}

	// Set material
	mMatList[_index] = _matName;
}

void MeshRenderer::SetIsRendering(bool _isRendering)
{
	mIsRendering = _isRendering;
}

void MeshRenderer::AddMatInstance(Material const& _mat)
{
	mMatInstanceList.push_back(_mat);
}

void MeshRenderer::ClearMaterialList(void)
{
	mMatInstanceList.clear();
}

//RTTR_REGISTRATION
//{
//	rttr::registration::class_<MeshRenderer>("MeshRenderer")
//	.constructor<>()
//	.property("Model", &MeshRenderer::GetModel, &MeshRenderer::SetModel)
//	.property("Is Rendering", &MeshRenderer::IsRendering, &MeshRenderer::SetIsRendering);
//}
