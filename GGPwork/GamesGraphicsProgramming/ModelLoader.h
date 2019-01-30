#pragma once

#include <windows.h>
#include <d3d11.h>

//Creation of Model Class
class Model {

public: 
	ID3D11Buffer *MVertexBuffer; //holds vertex buffer
	ID3D11ShaderResourceView *MTexture;//Holding Texture
	//store location and rotation
	float MPosX, MPosY, MPosZ, MRotX, MRotY, MRotZ;
	//Storing Scale
	float MScaleX, MScaleY, MScaleZ;
	int NumVerticles;
	//stores all filepaths
	const char *MeshFilepath;
	const wchar_t *TextFilepath;

	Model(const char *MFP, const wchar_t *TFP,float x,float y, float z);

	bool Load(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext);//needs to be added to load meshes
	
	//allows quicker setting of Rotation rather than changing xyz indvidually
	void SetRot(float x,float y, float z){
		MRotX = x;
		MRotY = y;
		MRotZ = z;
	}
	//allows quicker setting of Scale rather than chanign xyz indivudually
	void SetScale(float x, float y, float z) {
		MScaleX = x;
		MScaleY = y;
		MScaleZ = z;
	}
};
