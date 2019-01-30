#pragma once

#include <windows.h>
#include "StructHolder.h"
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "DirectXTK.lib")


class Billboard {
public:
	ID3D11Buffer *billboardVertexBuffer = NULL;//holding vertex buffer
	ID3D11ShaderResourceView *billboardImageData;
	const wchar_t *TextFilepath;//filepath for texture
	float billboardX;
	float billboardY;
	float billboardZ;
	float billboardRot;


	Billboard(const wchar_t *TFP, float x, float y, float z);

	bool Load(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext, HRESULT hr);

	bool Update(ID3D11DeviceContext *d3dContext, float CamPosX, float CamPosY);

	int GetNumVerticesInBillboardMesh();

};