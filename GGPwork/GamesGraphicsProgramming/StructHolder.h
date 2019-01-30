#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#ifndef STRUCTHOLDER_H
#define STRUCTHOLDER_H

using namespace DirectX;

struct Vertex {
	XMFLOAT3 MPos;
	XMFLOAT2 MTexCoord;

	//Constructor
	Vertex() {

	}
	//defgault COnstructor
	Vertex(float x, float y, float z, float u, float v) {
		MPos.x = x;
		MPos.y = y;
		MPos.z = z;
		MTexCoord.x = u;
		MTexCoord.y = v;
	}
};
struct WaterVertex {
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;

	WaterVertex(XMFLOAT3 vert, XMFLOAT2 uv) {
		pos = vert;
		texCoord = uv;
	}

	WaterVertex() {
	}
};
//Template for the terrain vertex and colour
struct VertexTerrain {
	XMFLOAT3 Pos;
	XMFLOAT2 TexCoord;

	//Constructor
	VertexTerrain(XMFLOAT3 xyz, XMFLOAT2 uv) {
		Pos = xyz;
		TexCoord = uv;
	}
	//defgault COnstructor
	VertexTerrain() {
		Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		TexCoord = XMFLOAT2(0.0f, 0.0f);
	}
};
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};
struct  BillboardVertex {
	XMFLOAT3 pos;
	XMFLOAT2 BBtexCoord;

};

#endif