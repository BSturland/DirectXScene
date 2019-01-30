#define NOMINMAX
#include <windows.h>
#include "BillboardLoader.h"
#include "StructHolder.h"
#include <WICTextureLoader.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "DirectXTK.lib")

BillboardVertex billboard_mesh[] = {
{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
};;




Billboard::Billboard(const wchar_t *TFP, float x, float y, float z) {
 
	TextFilepath = TFP;//filepath for texture
	billboardX = x;
	billboardY = y;
	billboardZ = z;
	
};

bool Billboard::Load(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,HRESULT hr) {

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, TextFilepath, 0, &billboardImageData, 0);

	//Create the Billboard vertex buffer object
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(BillboardVertex) * ARRAYSIZE(billboard_mesh);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = billboard_mesh;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &billboardVertexBuffer);


	return true;

}
bool Billboard::Update(ID3D11DeviceContext *d3dContext, float CamPosX, float CamPosZ) {
	billboardRot = atan2(billboardX - CamPosX, billboardZ - CamPosZ);
	d3dContext->PSSetShaderResources(1, 1, &billboardImageData);
	return true;
}

int Billboard::GetNumVerticesInBillboardMesh() {
	return ARRAYSIZE(billboard_mesh);
}