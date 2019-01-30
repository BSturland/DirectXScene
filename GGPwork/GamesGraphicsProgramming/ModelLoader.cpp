#define NOMINMAX
#include "ModelLoader.h"
#include "StructHolder.h"
#include <WICTextureLoader.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <DirectXMath.h>

Model::Model(const char *MFP, const wchar_t *TFP, float x, float y, float z) {
	MeshFilepath = MFP;
	TextFilepath = TFP;
	MPosX = x;
	MPosY = y;
	MPosZ = z;


}

bool Model::Load(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext) {
	//create importer
	Assimp::Importer imp;
	//Load meodle into scene object Convert to left handed is to stop meshes being back to front and inside out
	const aiScene *pScene = imp.ReadFile(MeshFilepath, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);


	//calculate all verticles 
	int TotalNoVerticles = 0;
	for (int MeshIndex = 0; MeshIndex < pScene->mNumMeshes; MeshIndex++) {
		const aiMesh *mesh = pScene->mMeshes[MeshIndex];
		TotalNoVerticles += mesh->mNumFaces * 3;
	}
	//store this number for drawing later
	NumVerticles = TotalNoVerticles;
	//accomidate all verticles
	Vertex *shape = new Vertex[TotalNoVerticles];

	int VerTexCount = 0;
	for (int MeshIndex = 0; MeshIndex < pScene->mNumMeshes; MeshIndex++) {
		const aiMesh *mesh = pScene->mMeshes[MeshIndex];
		//extract vertex from mesh main vertex
		for (int FaceIndex = 0; FaceIndex < mesh->mNumFaces; FaceIndex++) {
			const aiFace& Face = mesh->mFaces[FaceIndex];
			//extract a vertex from meshes main vertex array
			for (int VertexIndex = 0; VertexIndex < 3; VertexIndex++) {
				//extract pos and tex coridinates
				const aiVector3D *pos = &mesh->mVertices[Face.mIndices[VertexIndex]];
				const aiVector3D *tex = &mesh->mTextureCoords[0][Face.mIndices[VertexIndex]];
				//create a new object in the shape array
				shape[VerTexCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
				VerTexCount++;
			}
		}
	}

	//define Vertex shader
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * TotalNoVerticles;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	// Define the source of vertex data in RAM
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &MVertexBuffer);

	//Release shape memory
	delete[] shape;

	hr = DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, TextFilepath, NULL, &MTexture);

	return true;

}