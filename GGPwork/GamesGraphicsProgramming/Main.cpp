#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>
#include <list>
#include <XInput.h>
#include <WICTextureLoader.h>
#include <list>

#include "keyprocessor.h"
#include "StructHolder.h"
#include "BillboardLoader.h"
#include "ModelLoader.h"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>


//fixes fopen
#pragma warning (disable : 4996)


using namespace DirectX;
using namespace std;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")

// This example applies a texture to the shape

// Window class name
static wchar_t szAppClassName[] = L"GAMEGRAPHICSPROGRAMMING";

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// DirectX objects
ID3D11Device *d3dDevice;
ID3D11DeviceContext *d3dContext;
IDXGISwapChain *swapChain;
ID3D11RenderTargetView *backBuffer;
ID3D11RasterizerState *rs;

ID3D11Texture2D* RenderTargetTextureMap;
ID3D11RenderTargetView* RenderTargetViewMap;
ID3D11ShaderResourceView* ShaderResourceVIewMap;




XMFLOAT3 skybox[] = {

	//frontFace
	XMFLOAT3(-1.0f,-1.0f,-1.0f),
	XMFLOAT3(-1.0f,1.0f,-1.0f),
	XMFLOAT3(1.0f,-1.0f,-1.0f),

	XMFLOAT3(-1.0f,1.0f,-1.0f),
	XMFLOAT3(1.0f,1.0f,-1.0f),
	XMFLOAT3(1.0f,-1.0f,-1.0f),


	// Right side face
	 XMFLOAT3(1.0f, 1.0f, -1.0f),
	 XMFLOAT3(1.0f, 1.0f, 1.0f),
	 XMFLOAT3(1.0f, -1.0f, 1.0f),

	 XMFLOAT3(1.0f, -1.0f, 1.0f),
	 XMFLOAT3(1.0f, -1.0f, -1.0f),
	 XMFLOAT3(1.0f, 1.0f, -1.0f),

	 // Back face, note that points are in counter clockwise order
	  XMFLOAT3(-1.0f, -1.0f, 1.0f),
	  XMFLOAT3(1.0f, -1.0f, 1.0f),
	  XMFLOAT3(1.0f, 1.0f, 1.0f),

	  XMFLOAT3(1.0f, 1.0f, 1.0f),
	  XMFLOAT3(-1.0f, 1.0f, 1.0f),
	  XMFLOAT3(-1.0f, -1.0f, 1.0f),

	  //// Left side face, note that points are in counter clockwise order
	   XMFLOAT3(-1.0f, 1.0f, 1.0f),
	   XMFLOAT3(-1.0f, 1.0f, -1.0f),
	   XMFLOAT3(-1.0f, -1.0f, -1.0f),

	   XMFLOAT3(-1.0f, -1.0f, -1.0f),
	   XMFLOAT3(-1.0f, -1.0f, 1.0f),
	   XMFLOAT3(-1.0f, 1.0f, 1.0f),

	   //// Top face
		XMFLOAT3(-1.0f, 1.0f, -1.0f),
		XMFLOAT3(-1.0f, 1.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 1.0f),

		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, -1.0f),
		XMFLOAT3(-1.0f, 1.0f, -1.0f),

		//// Bottom face, note that points are in counter clockwise order
		 XMFLOAT3(-1.0f, -1.0f, -1.0f),
		 XMFLOAT3(1.0f, -1.0f, -1.0f),
		 XMFLOAT3(1.0f, -1.0f, 1.0f),

		 XMFLOAT3(1.0f, -1.0f, 1.0f),
		 XMFLOAT3(-1.0f, -1.0f, 1.0f),
		 XMFLOAT3(-1.0f, -1.0f, -1.0f),
};




// Stores the cubemap texture
// Stores the cube vertex buffer onto which the cubemap texture is put
ID3D11Buffer *skyboxVertexBuffer = NULL;
// Skybox vertex and pixel shaders.  These shaders are different
// to the shaders we've used so far so have to load them separately
ID3D11VertexShader *skyboxVertexShader = NULL;
ID3D11PixelShader *skyboxPixelShader = NULL;


//[2] Skybox DirectX resources
ID3D11ShaderResourceView *CubeMap;
ID3D11InputLayout *skyboxVertexLayout;
//rasterizer states turning off ba we see the texture of the cubemap
ID3D11RasterizerState *rsCullingOff;
ID3D11RasterizerState *rsCullingOn;
//depth test state enavler
ID3D11DepthStencilState* DSDepthOff;
ID3D11DepthStencilState* DSDepthOn;

// Depth buffer texture and object
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView* depthStencilView;

//Billboard Shader and layouts
ID3D11VertexShader *billboardVertexShader;
ID3D11PixelShader *billboardPixelShader;
ID3D11InputLayout *billboardVertexLayout = NULL;

// Triangle DirectX data/objects
ID3D11Buffer *constantBuffer = NULL;
ID3D11VertexShader *vertexShader = NULL;
ID3D11PixelShader *pixelShader = NULL;
ID3D11InputLayout *vertexLayout = NULL;

// This DirectX object stores the data from the background texture file
// This DirectX object is used to map texture data onto triangle
ID3D11SamplerState *sampler;

// Terrain data
//Terrain vertex & Index buffers
ID3D11Buffer *TerrainVertexBuffer = NULL;
ID3D11Buffer *TerrainIndexBuffer = NULL;
//No of indices in index buffer terrain triangles
int NoIndices;
//Terrain texture
ID3D11ShaderResourceView *TerrainTex;

// MVP Matrices
XMMATRIX matWorld;
XMMATRIX matView;
XMMATRIX matProjection;
XMMATRIX OrthoView;
XMMATRIX  OrthoProjection;

// Last frame time
DWORD frameTime;

// Camera position and Rotation
float CamPosX = 0.0f;
float CamPosY = 30.0f;
float CamPosZ = 0.0f;
float CamRotX = 0.0f;
float CamRotY = 0.0f;
float DotX = 0.0f;
float DotY = 0.0f;

// Camera target (lookat) and up direction (Y axis)
XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

//Orthographic Camera for later use on the HUD
float OrthoCamX;
float OrthoCamY;
float OrthoCamZ;
XMVECTOR OrthoPos = XMVectorSet(0.0, 0.0, -1.0,0.0f);
XMVECTOR OrthoTarget = XMVectorSet(0.0, 0.0, 0.0,0.0f);
XMVECTOR OrthoUp = XMVectorSet(0.0, 1.0, 0.0,0.0f);

ID3D11BlendState* g_pBlendState = NULL;

// Store extracted mouse position to help work out rotation
int curMouseXPos, curMouseYPos;

// Stores the mid point of the client area, going to use this to compute the movement from centre of window
int clientMidX, clientMidY;

// Store the windows position and mid point in order to reset mouse position
int winMidX, winMidY;
int winLeft, winTop;

//Use to store computed rotation factor
float rotFactorY = 0.0f;

//WATER
ID3D11Buffer *waterVertexBuffer = NULL;
ID3D11ShaderResourceView *WaterImageData;
ID3D11ShaderResourceView *BottomImageData;
ID3D11PixelShader *WaterPixelShader = NULL;
WaterVertex *WaterShape;
int WaterNumVert;
float Displacement = 1.0f;
float Rotation = 0.0f;






// Initialise DirectX in the application
BOOL InitialiseDX(HWND hMainWnd, HINSTANCE hCurInstance) {
	RECT rectDimensions;
	GetClientRect(hMainWnd, &rectDimensions);

	LONG width = rectDimensions.right - rectDimensions.left;
	LONG height = rectDimensions.bottom - rectDimensions.top;

	// Get the Rectangle of the window relative to the desktop 
	RECT rectWindow;
	GetWindowRect(hMainWnd, &rectWindow);

	// Storing Top left of rectangle To set the cursosr to middle of the windows
	winTop = rectWindow.top;
	winLeft = rectWindow.left;

	// Workout the middleof the window relative to the desktop
	winMidX = (rectWindow.right - rectWindow.left) / 2;
	winMidY = (rectWindow.bottom - rectWindow.top) / 2;

	// Calculate the middle of the client area in order to calculate the change in mouse position
	clientMidX = width / 2;
	clientMidY = height / 2;

	// Initialise mouse position variables to mid point of client area
	curMouseXPos = clientMidX;
	curMouseYPos = clientMidY;

	// Set the cursor to the middle of the window.  Will calculate change in Update
	SetCursorPos(winLeft + winMidX, winTop + winMidY);

	// Calculate the rotation factor with respect to the width
	rotFactorY = XM_PIDIV2 / width;

	// Define the feature levels the program 
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };

	int numFeatureLevels = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);

	// Setup the DXGI_SWAP_CHAIN_DESC structure for describing the type of swap chain to be used
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hMainWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	int creationFlags = 0;

	HRESULT result;

	// Create the device and swap chain for DirectX 11
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &d3dDevice, NULL, &d3dContext);

	if (result != S_OK) {
		MessageBox(hMainWnd, L"Failed to initialise DX11!", szAppClassName, NULL);

		return false;
	}

	// Create a texture to be used as the back buffer (off screen)
	ID3D11Texture2D *backBufferTexture;

	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBufferTexture);

	if (result != S_OK) {
		MessageBox(hMainWnd, L"Failed to get back buffer!", szAppClassName, NULL);

		return false;
	}

	result = d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &backBuffer);

	if (backBufferTexture != NULL) {
		backBufferTexture->Release();
	}

	if (result != S_OK) {
		MessageBox(hMainWnd, L"Failed to get render target!", szAppClassName, NULL);

		return false;
	}

	//Define Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));

	// DepthStencil buffer has same dimensions as back buffer which is the same as the client window
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	// Major difference is depth buffer does not store colour information but depth information
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	// As it's a texture need to define DirectX usage, would be useful to use D3D11_USAGE_DYNAMIC flag and caputre
	// state of depth buffer to examine the values stored.
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// Create the texture for the depthstencil buffer
	d3dDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	// Create the depthstencil object based on the texture created precious
	d3dDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	// Bind the back buffer and the depthstencil buffer to the OM stage
	d3dContext->OMSetRenderTargets(1, &backBuffer, depthStencilView);

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	d3dContext->RSSetViewports(1, &viewport);

	// Initialize the view matrix based on camera position
	XMVECTOR camPos = XMVectorSet(CamPosX, CamPosY, CamPosZ, 0.0f);
	matView = XMMatrixLookAtLH(camPos, camTarget, camUp);

	OrthoView = XMMatrixLookAtLH(OrthoPos, OrthoTarget, OrthoUp);
	OrthoProjection = XMMatrixOrthographicLH(width,height, 0.001f, 500.0f);

	// Initialize the projection matrix
	matProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2 / 2.0f, width / (FLOAT)height, 0.01f, 500.0f);

	return true;
}


bool CreateShaders(HWND hMainWnd) {
	// Createing shaders, Order doesnt matter here, only that they are put into DX objects
	ID3DBlob *pVSBlob = NULL;
	HRESULT hr = D3DReadFileToBlob(L".\\VertexShader.cso", &pVSBlob);
	if (hr != S_OK)
	{
		MessageBox(hMainWnd, L"Problem loading vertex shader.  Check shader file (VertexShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", szAppClassName, MB_OK);

		return false;
	}

	// Create the vertex shader in DirectX
	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader);
	// Check creating vertex shader was successful
	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, L"The vertex shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}

	// Need to tell DirectX how vertices are structured in terms of colour format and order of data, that is individual vertices
	// The POSITION is called the semantic name, basically a meaningful identifier used internally to map vertex elements to shader 
	// parameters.  Semantic name text must obey the rules of C identifiers as shader language uses C syntax
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = 2;

	// Create the input layout for input assembler based on description and the vertex shader to be used
	hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &vertexLayout);
	// Check layout was created successfully
	pVSBlob->Release();
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Problems creating input layout", szAppClassName, MB_OK);

		return false;
	}

	// Set the input layout for input assembler
	d3dContext->IASetInputLayout(vertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = D3DReadFileToBlob(L".\\PixelShader.cso", &pPSBlob);
	// Check pixel shader was loaded successfully
	if (hr != S_OK)
	{
		MessageBox(hMainWnd, L"Problem loading pixel shader.  Check shader file (PixelShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", szAppClassName, MB_OK);

		return false;
	}

	// Create the pixel shader in DirectX
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader);
	pPSBlob->Release();
	// Check creating pixel shader was successful
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The pixel shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}

	//terrain texture
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\Terrain.jpg", 0, &TerrainTex, 0);


	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", szAppClassName, MB_OK);

		return false;
	}

	//Skybox vertex shader cfrewation
	hr = D3DReadFileToBlob(L".\\SkyboxVertexShader.cso", &pVSBlob);
	if (hr != S_OK)
	{
		MessageBox(hMainWnd, L"Problem loading skybox vertex shader.  Check shader file (SkyboxVertexShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", szAppClassName, MB_OK);

		return false;
	}

	// Define the input layout for the skybox shader
	// Need to tell DirectX how vertices are structured for skybox.  Major difference is there's
	// no texture information as the position is used as the texture co-ord, kind of like a
	// 3D texture co-ord
	D3D11_INPUT_ELEMENT_DESC skybox_layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0, D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	numElements = 1;
	// Create the input layout for input assembler based on description and the vertex shader to be used
	hr = d3dDevice->CreateInputLayout(skybox_layout,
		numElements,
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &skyboxVertexLayout);
	// Check layout was created successfully
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Problem creating skybox layout", szAppClassName, MB_OK);
		return false;
	}

	// Release the vertex shader source data, Load the skybox pixel shader, Compile the pixel shader
	
	hr = D3DReadFileToBlob(L".\\SkyboxPixelShader.cso", &pPSBlob);

	// Create the vertex shader in DirectX
	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &skyboxVertexShader);
	// Check creating vertex shader was successful
	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, L"The skybox vertex shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}
	// Create the pixel shader in DirectX
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &skyboxPixelShader);
	pPSBlob->Release();
	// Check creating pixel shader was successful
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The skybox pixel shader object cannot be created", szAppClassName, MB_OK);

		return false;
	}

	// Create the sampler
	// First step is to create the D3D11_SAMPLER_DESC in which
	// we set fields to describe how the texture will be mapped onto
	// the mesh using the uv texcoords defined in the mesh
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the sampler.
	hr = d3dDevice->CreateSamplerState(&sampDesc, &sampler);
	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create sampler"), szAppClassName, MB_OK);

		return false;
	}

	/*
	WATER
	*/

	//LoadWater Shader to memory
	//pPSBlob = NULL;
	hr = D3DReadFileToBlob(L".\\WaterPixelShader.cso", &pPSBlob);
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &WaterPixelShader);
	pPSBlob->Release();
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The pixel shader object cannot be created",
			szAppClassName, MB_OK);

		return false;
	}
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\Resources\\water.jpg", 0, &WaterImageData, 0);
	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\Resources\\stones.jpg", 0, &BottomImageData, 0);

	// If we've got this far then have valid shaders.
	// Return a successful result
	return true;
}


void LoadHeightMap(const char *filename, float MaxHeight, VertexTerrain **mesh) {
	// This expects a texture file of 256 pixels width and height
	// Also presumes height map is 24 bit texture file
	const int Size = 256;

	FILE *file_ptr = fopen(filename, "rb");

	if (file_ptr == NULL) {
		return;
	}

	/* Read the file's header information
	 so we can create a buffer to hold the pixel colour data*/
	BITMAPFILEHEADER bmHeader;
	BITMAPINFOHEADER bmInfo;

	fread(&bmHeader, sizeof(BITMAPFILEHEADER), 1, file_ptr);

	fread(&bmInfo, sizeof(BITMAPINFOHEADER), 1, file_ptr);

	// Move file pointer to start of pixel colour data
	fseek(file_ptr, bmHeader.bfOffBits, SEEK_SET);

	// Work out number of bytes per pixel for array
	int NumBytesPerPixel = bmInfo.biBitCount / 8;

	// Calculate maximum colour value up 2^24 colour
	float MaxColourVal = (float)pow(2.0f, min(24.0f, (float)bmInfo.biBitCount));

	// Create array for pixel data
	unsigned char *pixel_data = new unsigned char[Size * Size * NumBytesPerPixel];

	// Read pixel data from file into memory
	fread(pixel_data, sizeof(unsigned char), Size * Size * NumBytesPerPixel, file_ptr);

	// Release the file
	fclose(file_ptr);

	//[13] Modify each vertex's y component based on the pixel value in proportion to max colour value
	// in the corresponding pixel
	int idx = 0;
	VertexTerrain *VertArray = *mesh;
	for (int row = 0; row < Size; row++) {
		for (int col = 0; col < Size; col++) {
			//depends on data being 2 bit
			int pixel_val = pixel_data[idx * NumBytesPerPixel] +
				(pixel_data[(idx * NumBytesPerPixel) + 1] << 8) +
				(pixel_data[(idx * NumBytesPerPixel) + 2] << 16);

			VertArray[idx].Pos.y = MaxHeight * (pixel_val / MaxColourVal);
			idx++;
		}
	}


	// Release pixel data as don't need it any more
	delete[] pixel_data;
}

//Create terrain mesh based on the number of vertices both horizontally and depth.
void CreateTerrainMesh(int NoVerticles, VertexTerrain **Mesh, int *TotalNoVert, int **Indices, int *NoIndices)
{
	//Calculate the total no vertices and store in array
	*TotalNoVert = NoVerticles * NoVerticles;
	*Mesh = new VertexTerrain[*TotalNoVert];
	//Calculate step between vertices
	float step = 2.0f / NoVerticles;

	// Starting point of terrain -1.0f, 0.0f, -1.0f
	float XPos = -1.0f;
	float ZPos = -1.0f;

	//Calculate the uv step
	float uvStep = 1.0f / NoVerticles;


	// Starting point of uv 0.0f, 0.0f
	float u = 0.0f;
	float v = 0.0f;

	int idx = 0;
	VertexTerrain *VertArray = *Mesh;

	//Create vertices for terrain based on NumVertices
	for (int row = 0; row < NoVerticles; row++) {
		for (int col = 0; col < NoVerticles; col++) {
			VertArray[idx] = VertexTerrain(XMFLOAT3(XPos, 0.0f, ZPos), XMFLOAT2(u, v));
			//move by step amouts to next vertexc position
			XPos += step;
			u += uvStep;
			idx++;
		}
		u = 0.0f;
		v += uvStep;
		XPos = -1.0f;
		ZPos += step;
	}

	//Create index array.  Note that the number of indices
	// are reflective of the number of triangles not lines
	// in grid, hence 1 less than the number of lines
	*NoIndices = (NoVerticles - 1) * (NoVerticles - 1) * 6;
	*Indices = new int[*NoIndices];

	int *p = *Indices;

	// Define the first two triangles
	p[0] = 0;
	p[1] = NoVerticles;
	p[2] = NoVerticles + 1;
	p[3] = NoVerticles + 1;
	p[4] = 1;
	p[5] = 0;


	//creates the other triangles
	int NextRow = (NoVerticles - 1) * 6;
	for (int idx = 6; idx < *NoIndices; idx++) {
		//deal with corner case
		if (idx % NextRow == 0) {
			p[idx] = (idx / NextRow) * NoVerticles;
			p[idx + 1] = p[idx] + NoVerticles;
			p[idx + 2] = p[idx + 1] + 1;
			p[idx + 3] = p[idx + 2];
			p[idx + 4] = p[idx]+1;
			p[idx + 5] = p[idx];

			idx += 5;
		}
		else {
			p[idx] = p[idx - 6] + 1;
		}
	}


}
/*

MODELS

*/

Model Ship(".\\Resources\\Space_frigate_6.obj", L".\\Resources\\space_frigate_6_color.jpg",0.0f,0.0f,0.0f);

Model Sydney(".\\Resources\\Sydney.md2",L".\\Resources\\sydney.bmp",0.0f,0.0f,0.0f);

Model Earth(".\\Resources\\Earth.x",L".\\Resources\\earth.jpg",0.0f,0.0f,0.0f);

Model PC(".\\Resources\\PC.obj", L".\\Resources\\PC.jpg", 0.0f, 0.0f, 0.0f);

Model Container(".\\Resources\\Container.obj", L".\\Resources\\Container.jpg", 0.0f, 0.0f, 0.0f);
/*

BILLBOARDS

*/

Billboard Test(L".\\Resources\\Kanji.png",-50.0f,10.0f,10.0f);
Billboard AdVert(L".\\Resources\\KokeAd.png", -50.0f, 10.0f, 30.0f);
Billboard SecCam(L".\\Resources\\Camera.png", -50.0f, 10.0f, -30.0f);
Billboard Smile(L".\\Resources\\Smileyface.png", -50.0f, 10.0f, -40.0f);
Billboard AdVert2(L".\\Resources\\SpritAd.png", -50.0f, 10.0f, -50.0f);

/*
UI TEST
*/
Billboard UISmile(L".\\Resources\\Smileyface.png", -50.0f, 10.0f, -40.0f);
Billboard HUD(L".\\Resources\\HUD.png", -50.0f, 10.0f, -40.0f);
Billboard Map(L".\\Resources\\Map.png",0.0f,0.0f,0.0f);
Billboard Dot(L".\\Resources\\DOT.png", 0.0f, 0.0f, 0.0f);




// Loads texture data from a file and copied it into a face of the cubemap specified by faceIdx
bool LoadFace(const wchar_t *filename, int faceIdx, ID3D11Texture2D *cubemapTex) {
	// Load the texture into a memory
	ID3D11ShaderResourceView *faceData;
	ID3D11Resource *faceRes;
	HRESULT hr = CreateWICTextureFromFile(d3dDevice, d3dContext, filename, &faceRes, &faceData, 0);

	if (hr != S_OK) {
		return false;
	}

	// Get the texture object from the shader resource loaded
	ID3D11Texture2D *tex;

	hr = faceRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID *)&tex);

	if (hr != S_OK) {
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	tex->GetDesc(&texDesc);

	// Copy texture from shader resource to cubemap face specified by faceIdx
	D3D11_BOX srcRegion;

	srcRegion.front = 0;
	srcRegion.back = 1;
	srcRegion.top = 0;
	srcRegion.left = 0;
	srcRegion.bottom = texDesc.Height;
	srcRegion.right = texDesc.Width;

	// Determine the subresource object corresponding to the face id
	int face = D3D11CalcSubresource(0, faceIdx, 1);
	d3dContext->CopySubresourceRegion(cubemapTex, face, 0, 0, 0, faceRes, 0, &srcRegion);

	// Release the face texture object
	faceData->Release();

	return true;
}


bool CreateCubemapSkybox(const wchar_t *up_fname,
	const wchar_t *down_fname,
	const wchar_t *left_fname,
	const wchar_t *right_fname,
	const wchar_t *front_fname,
	const wchar_t *back_fname,
	int lengthOfSide)
{
	//Define the texture comprising of 6 textures and width and height = lengthOfSide
	D3D11_TEXTURE2D_DESC cubemapDesc;
	ZeroMemory(&cubemapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	cubemapDesc.Width = lengthOfSide;
	cubemapDesc.Height = lengthOfSide;
	cubemapDesc.MipLevels = 1;
	cubemapDesc.ArraySize = 6;
	cubemapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubemapDesc.SampleDesc.Count = 1;
	cubemapDesc.SampleDesc.Quality = 0;
	cubemapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cubemapDesc.CPUAccessFlags = 0;
	cubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	//Create the cubemap texture object
	ID3D11Texture2D *cubemapBuffer;
	// Create the texture for the depthstencil buffer
	HRESULT hr = d3dDevice->CreateTexture2D(&cubemapDesc, nullptr, &cubemapBuffer);
	if (hr != S_OK) {
		return false;
	}
	//load front face
	LoadFace(front_fname, 4, cubemapBuffer);
	LoadFace(right_fname, 0, cubemapBuffer);
	LoadFace(left_fname, 1, cubemapBuffer);
	LoadFace(up_fname, 2, cubemapBuffer);
	LoadFace(down_fname, 3, cubemapBuffer);
	LoadFace(back_fname, 5, cubemapBuffer);






	//Convert cubemap texture into shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	hr = d3dDevice->CreateShaderResourceView(cubemapBuffer, &srDesc, &CubeMap);
	if (hr != S_OK) { return false; }
	cubemapBuffer->Release();
	//Creates the skybox mesh
	// Define the triangle in terms of the three points at each corner
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(XMFLOAT3)*ARRAYSIZE(skybox);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	// vertexBuffer is a global variable that points to the DirectX vertex buffer created for program
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	// Needs access to the triangle structure defined above
	InitData.pSysMem = skybox;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &skyboxVertexBuffer);
	if (hr != S_OK) { return false; }


	return true;
}

void CreateWaterVerticles(float Size, WaterVertex **shape, int *WaterNumVert) {
	//dimesnion of mesh
	const int Length = 10;
	//store dynamically created verticles
	list<WaterVertex> vertices;
	//define calclated resolution
	float Res = Size / Length;
	float Offset = Size / 2;
	float uvStep = 1.0f / Size;
	//define varibles for calcuating verts and uvs
	float x = -Offset;
	float y = 0.0;
	float z = -Offset;
	float u = 0.0f;
	float v = 1.0f;

	while (z < +Offset) {
		//do a row of triangles
		while (x < +Offset) {
			vertices.push_back(WaterVertex(XMFLOAT3(x, y, z), XMFLOAT2(u, v)));
			vertices.push_back(WaterVertex(XMFLOAT3(x, y, z + Res), XMFLOAT2(u, v - uvStep)));
			vertices.push_back(WaterVertex(XMFLOAT3(x + Res, y, z), XMFLOAT2(u + uvStep, v)));

			vertices.push_back(WaterVertex(XMFLOAT3(x, y, z + Res), XMFLOAT2(u, v - uvStep)));
			vertices.push_back(WaterVertex(XMFLOAT3(x +Res, y, z), XMFLOAT2(u + uvStep, v - uvStep)));
			vertices.push_back(WaterVertex(XMFLOAT3(x + Res, y, z), XMFLOAT2(u +uvStep, v)));

			x += Res;
			u += uvStep;
		}
		//updatenextrow
		v -= uvStep;
		u = 0.0f;
		x = -Offset;
		z += Res;
	}
	//copy verticles into block of memory which creates vertex buffer
	*WaterNumVert = vertices.size();
	*shape = new WaterVertex[vertices.size()];
	int idx = 0;
	for (list<WaterVertex>::iterator it = vertices.begin(); it != vertices.end(); it++) {
		(*shape)[idx] = *it;
		idx++;
	}



}

bool CreateMeshes(HWND hMainWnd) {
	

	//variables store terrain vertex&index
	VertexTerrain *TerrainVerticles;
	int *TerrainIndices;
	int NoVert;

	//LOADING MODELS
	Ship.Load(d3dDevice,d3dContext);
	Sydney.Load(d3dDevice, d3dContext);
	Earth.Load(d3dDevice, d3dContext);
	PC.Load(d3dDevice, d3dContext);
	Container.Load(d3dDevice, d3dContext);

	//Create the terrain mesh
	CreateTerrainMesh(256, &TerrainVerticles, &NoVert, &TerrainIndices, &NoIndices);
	LoadHeightMap(".\\HeightMap.bmp", 10.0f, &TerrainVerticles);

	//[vertex buffer for Terrain
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(VertexTerrain)*NoVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// vertexBuffer is a global variable that points to the DirectX vertex buffer created for program
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	//Needs access to triangle strucute
	InitData.pSysMem = TerrainVerticles;
	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &TerrainVertexBuffer);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create vertex buffer", szAppClassName, NULL);

		return false;
	}
	/*
	BILLBOARD>LOAD HERE
	*/

	Test.Load(d3dDevice, d3dContext, hr);
	AdVert.Load(d3dDevice, d3dContext, hr);
	SecCam.Load(d3dDevice, d3dContext, hr);
	Smile.Load(d3dDevice, d3dContext, hr);
	AdVert2.Load(d3dDevice, d3dContext, hr);
	UISmile.Load(d3dDevice, d3dContext, hr);
	HUD.Load(d3dDevice, d3dContext, hr);
	Map.Load(d3dDevice, d3dContext, hr);
	Dot.Load(d3dDevice, d3dContext, hr);



	/*
WATER
*/
	CreateWaterVerticles(10, &WaterShape, &WaterNumVert);
	//D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;;
	bd.ByteWidth = sizeof(WaterVertex)*WaterNumVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//making buffer accessible to CPU
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//create vertex buffer
	D3D11_SUBRESOURCE_DATA WaterInitdata;
	ZeroMemory(&WaterInitdata, sizeof(WaterInitdata));
	//needs access to triangle data
	WaterInitdata.pSysMem = WaterShape;
	hr = d3dDevice->CreateBuffer(&bd, &WaterInitdata, &waterVertexBuffer);



	//Index buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(int)*NoIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	// Needs access to the triangle structure defined in struct holder
	InitData.pSysMem = TerrainIndices;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &TerrainIndexBuffer);


	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create index buffer", szAppClassName, NULL);

		return false;
	}



	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, NULL, &constantBuffer);
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create constant buffer", szAppClassName, NULL);

		return false;
	}
	//Free memory allocated to arrays
	delete[] TerrainVerticles;
	delete[] TerrainIndices;




	// If we've got this far then have vertice structure in DirectX.
	// Return a successful result
	return true;
}

bool SetRasterizerState(HWND hMainWnd) {
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = true;

	HRESULT result = d3dDevice->CreateRasterizerState(&rasDesc, &rs);

	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), szAppClassName, NULL);
	}

	return true;
}

void Update() {
	
	DWORD now = GetTickCount();
	DWORD diff = now - frameTime;

	const float move_step = 20.0f;

	// Store the calculated amount of movement
	// note that there is no rotation variable as this is calculated from mouse movement
	float forwards_backwards = 0.0f;
	float up_down = 0.0f;
	float left_right = 0.0f;

	// Calculate the amount of movement forwards and backwards
	if (IsKeyDown('W')) {
		forwards_backwards = move_step * (diff / 1000.0f);
	}

	if (IsKeyDown('S')) {
		forwards_backwards = -move_step * (diff / 1000.0f);
	}
	if (IsKeyDown('D')) {
		left_right = move_step * (diff / 1000.0f);
	}

	if (IsKeyDown('A')) {
		left_right = -move_step * (diff / 1000.0f);
	}
	// Calculate the amount of movement forwards and backwards
	if (IsKeyDown('X')) {
		up_down = move_step * (diff / 1000.0f);
	}

	if (IsKeyDown('C')) {
		up_down = -move_step * (diff / 1000.0f);
	}

	// Calculate the change mouse X pos
	int deltaX = clientMidX - curMouseXPos;
	int deltaY = clientMidY - curMouseYPos -12;

	// Update the rotation 
	CamRotY -= deltaX * rotFactorY;
	CamRotX -= deltaY * rotFactorY;
	// Reset cursor position to middle of window
	SetCursorPos(winLeft + winMidX, winTop + winMidY);

	XMVECTOR oldCamTarget = camTarget;
	float oldCamX = CamPosX, oldCamY = CamPosY, oldCamZ = CamPosZ;

	// Calculate distance to move camera with respect to movement on Z axis and rotation
	XMMATRIX camMove = XMMatrixTranslation(0.0f, up_down, forwards_backwards) *
		XMMatrixRotationRollPitchYaw(CamRotX, CamRotY, 0.0f);

	// Obtain the translation from the move matrix
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, camMove);

	// Update the camera position X, Y and Z
	CamPosX += XMVectorGetX(trans);
	CamPosY += XMVectorGetY(trans);
	CamPosZ += XMVectorGetZ(trans);

	XMVECTOR camPos = XMVectorSet(CamPosX, CamPosY, CamPosZ, 0.0f);

	// Calculate the relative distance from the camera to look at with respect to distance and rotation
	XMMATRIX camDist = XMMatrixTranslation(0.0f, 0.0f, 10.0f) *
		XMMatrixRotationRollPitchYaw(CamRotX, CamRotY, 0.0f);

	// Obtain the translation and calculate target
	XMMatrixDecompose(&scale, &rot, &trans, camDist);

	//BILLOBARD UPDATES
	Test.Update(d3dContext, CamPosX, CamPosZ);
	AdVert.Update(d3dContext, CamPosX, CamPosZ);
	SecCam.Update(d3dContext, CamPosX, CamPosZ);
	Smile.Update(d3dContext, CamPosX, CamPosZ);
	AdVert2.Update(d3dContext, CamPosX, CamPosZ);

	// Calculate a new target to look at with respect to the camera position
	camTarget = XMVectorSet(CamPosX + XMVectorGetX(trans),
		CamPosY + XMVectorGetY(trans),
		CamPosZ + XMVectorGetZ(trans), 0.0f);

	matView = XMMatrixLookAtLH(camPos, camTarget, camUp);
	
	//working minimap out
	DotX = (CamPosX / 200) * 175;
	DotY = (CamPosZ / 200) * 175;
	
	/*WATER*/
	Rotation += (XM_PI / 2.0f) * (diff / 1000.0f);
	if (Rotation > 2.0f * XM_PI) { Rotation = 0.0f; }
	D3D11_MAPPED_SUBRESOURCE mappedData;
	d3dContext->Map(waterVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	WaterVertex *v = reinterpret_cast<WaterVertex*>(mappedData.pData);
	//changed y position and vertivle
	float xStart = v[0].pos.x;
	float zStart = v[0].pos.z;
	for (int count = 0; count < WaterNumVert; count++) {
		v[count] = WaterShape[count];

		v[count].pos.y += Displacement * XMScalarSin(Rotation + (2 * XM_PI) *
			((v[count].pos.x - xStart) / 10.0f)) +
			Displacement *
			XMScalarCos(Rotation + (2 * XM_PI)* ((v[count].pos.z - zStart) / 10.0f));
	}
	d3dContext->Unmap(waterVertexBuffer, 0);
	


	frameTime = now;
}

// Renders a frame, what does this function do?
void Draw() {
	if (d3dContext == NULL) {
		return;
	}

	//prepping to change blendstate
	ID3D11BlendState *old_state;
	FLOAT old_factor[4];
	UINT old_mask;
	d3dContext->OMGetBlendState(&old_state, old_factor, &old_mask);

	float colour[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	d3dContext->ClearRenderTargetView(backBuffer, colour);

	// Clear the depth of depthstencil buffer to 1.0f for new frame
	d3dContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	ConstantBuffer cb;
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	
	
	//DRAW SKYBOX
	
//Calculate WVP for skybox based on camera position
	matWorld = XMMatrixTranslation(CamPosX, CamPosY, CamPosZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	//Set the skybox shaders
	// Set up to render skybox, remember skybox has different shaders to
	// other models because it uses a cubemap
	d3dContext->VSSetShader(skyboxVertexShader, NULL, 0);
	d3dContext->PSSetShader(skyboxPixelShader, NULL, 0);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	// Select the pixel colour data and sampler to select pixel colours from texture
	// with respect to uv co-ordinates
	//Set the skybox cubemap texture
	d3dContext->PSSetShaderResources(1, 1, &CubeMap);
	//Set the input layout for the skybox vertex shader
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &skyboxVertexBuffer, &stride, &offset);
	//Turn off the depth test as cubebox will be drawn at maximum depth
	d3dContext->OMSetDepthStencilState(DSDepthOff, 0);
	//Turn off culling otherwise would not see inside of cube
	d3dContext->RSSetState(rsCullingOff);
	//Draw skybox
	d3dContext->Draw(ARRAYSIZE(skybox), 0);
	//Restore depth test and backface culling before drawing other objects
	d3dContext->RSSetState(rsCullingOn);
	d3dContext->OMSetDepthStencilState(DSDepthOn, 0);
	


	/*
WATER
*/

	d3dContext->VSSetShader(vertexShader, NULL, 0);
	d3dContext->PSSetShader(WaterPixelShader, NULL, 0);
	matWorld = XMMatrixScaling(5.0f, 1.0f, 5.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	stride = sizeof(WaterVertex);
	offset = 0;
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &WaterImageData);
	d3dContext->PSSetShaderResources(2, 1, &BottomImageData);
	d3dContext->Draw(WaterNumVert, 0);

	



	// Set vertex and pixel shaders, these are common  all meshes
	d3dContext->VSSetShader(vertexShader, NULL, 0);
	d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	d3dContext->PSSetShader(pixelShader, NULL, 0);

	// Select the sampler object to map colour data from texture to mesh.  Simple sampler common to all textures
	d3dContext->PSSetSamplers(0, 1, &sampler);

	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	

	d3dContext->IASetInputLayout(vertexLayout);
	//Transform terrain into scene and send to graphics device
	matWorld = XMMatrixScaling(100.0f, 5.0f, 100.0f) * XMMatrixTranslation(0,0,0);

	cb.mWorld = XMMatrixTranspose(matWorld);

	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	//Select the terrain vertex buffer
	stride = sizeof(VertexTerrain);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &TerrainVertexBuffer, &stride, &offset);

	//Input buffer
	d3dContext->IASetIndexBuffer(TerrainIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the image data from which the sampler will map onto mesh pixels
	d3dContext->PSSetShaderResources(1, 1, &TerrainTex);


	// Show in wireframe, comment out the following line to show solid
	//d3dContext->RSSetState(rs);


	//Draw the terrain
	d3dContext->DrawIndexed(NoIndices, 0, 0);



	/*
	MODELS
	*/
	
	matWorld = XMMatrixScaling(1.1f, 1.1f, 1.1f)* XMMatrixTranslation(0.0f, 6.0f, 50.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	cb.mView = XMMatrixTranspose(matView);
	cb.mProjection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->IASetVertexBuffers(0, 1, &Ship.MVertexBuffer, &stride, &offset);
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &Ship.MTexture);
	d3dContext->Draw(Ship.NumVerticles, 0);
	//





	//sdney
	matWorld = XMMatrixScaling(0.1f, 0.1f, 0.1f)* XMMatrixTranslation(-20.0f, 2.4f, 9.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);


	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->IASetVertexBuffers(0, 1, &Sydney.MVertexBuffer, &stride, &offset);

	d3dContext->PSSetShaderResources(1, 1, &Sydney.MTexture);

	d3dContext->Draw(Sydney.NumVerticles, 0);


	//Earth
	matWorld = XMMatrixScaling(10.1f, 10.1f, 10.1f)* XMMatrixTranslation(0.0f, 100.0f, 0.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);

	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->IASetVertexBuffers(0, 1, &Earth.MVertexBuffer, &stride, &offset);

	d3dContext->PSSetShaderResources(1, 1, &Earth.MTexture);

	d3dContext->Draw(Earth.NumVerticles, 0);
	
	//PC
	matWorld = XMMatrixScaling(0.04f, 0.04f, 0.04f)* XMMatrixTranslation(-10.0f, 5.0f, -10.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);

	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->IASetVertexBuffers(0, 1, &PC.MVertexBuffer, &stride, &offset);

	d3dContext->PSSetShaderResources(1, 1, &PC.MTexture);

	d3dContext->Draw(PC.NumVerticles, 0);
	
	//Container
	matWorld = XMMatrixScaling(0.03f, 0.03f, 0.03f)* XMMatrixTranslation(-15.0f, 4.0f, -40.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);

	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->IASetVertexBuffers(0, 1, &Container.MVertexBuffer, &stride, &offset);

	d3dContext->PSSetShaderResources(1, 1, &Container.MTexture);

	d3dContext->Draw(Container.NumVerticles, 0);

	//Setting transparent blendstate
	d3dContext->OMSetBlendState(g_pBlendState, 0, 0xffffffff);



	/*
	BILLBOARDS
	*/
	//set pixel and vertex stages of graphics piepline
	matWorld = XMMatrixRotationY(Test.billboardRot)* XMMatrixScaling(5.0f, 5.0f, 5.0f)* XMMatrixTranslation(Test.billboardX, Test.billboardY, Test.billboardZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &Test.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &Test.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(Test.GetNumVerticesInBillboardMesh(), 0);

	//Advert
	//set pixel and vertex stages of graphics piepline
	matWorld = XMMatrixRotationY(AdVert.billboardRot)* XMMatrixScaling(5.0f, 5.0f, 5.0f)* XMMatrixTranslation(AdVert.billboardX, AdVert.billboardY, AdVert.billboardZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &AdVert.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &AdVert.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(AdVert.GetNumVerticesInBillboardMesh(), 0);


	//SecCam
	matWorld = XMMatrixRotationY(SecCam.billboardRot)* XMMatrixScaling(0.5f, 0.5f, 0.5f)* XMMatrixTranslation(SecCam.billboardX, SecCam.billboardY, SecCam.billboardZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &SecCam.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);

	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &SecCam.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(SecCam.GetNumVerticesInBillboardMesh(), 0);


	//smile
	matWorld = XMMatrixRotationY(Smile.billboardRot)* XMMatrixScaling(0.5f, 0.5f, 0.5f)* XMMatrixTranslation(Smile.billboardX, Smile.billboardY, Smile.billboardZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &Smile.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);

	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &Smile.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(Smile.GetNumVerticesInBillboardMesh(), 0);


	//Advert2
//set pixel and vertex stages of graphics piepline
	matWorld = XMMatrixRotationY(AdVert2.billboardRot)* XMMatrixScaling(5.0f, 5.0f, 5.0f)* XMMatrixTranslation(AdVert2.billboardX, AdVert2.billboardY, AdVert2.billboardZ);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &AdVert2.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &AdVert2.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(AdVert2.GetNumVerticesInBillboardMesh(), 0);

	/*
	UI rendered last
	using orthographic camera
	*/
	//changing constant buffer to orthocrahpic cam
	cb.mView = XMMatrixTranspose(OrthoView);
	cb.mProjection = XMMatrixTranspose(OrthoProjection);

	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	matWorld = XMMatrixScaling(10.0f, 10.0f, 10.0f)* XMMatrixTranslation(0.0f,0.0f,0.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &UISmile.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &UISmile.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(UISmile.GetNumVerticesInBillboardMesh(), 0);

	//uiHUD
	matWorld = XMMatrixScaling(550.0f, 550.0f, 550.0f)* XMMatrixTranslation(10.0f, 0.0f, 0.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &HUD.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &HUD.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(HUD.GetNumVerticesInBillboardMesh(), 0);

	//map
	d3dContext->OMSetDepthStencilState(DSDepthOff, 0);
	//uiMap
	matWorld = XMMatrixScaling(100.0f, 100.0f, 100.0f)* XMMatrixTranslation(-400.0f, -200.0f, 0.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &Map.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &Map.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(Map.GetNumVerticesInBillboardMesh(), 0);
	
	//uiDot
	matWorld = XMMatrixScaling(100.0f, 100.0f, 100.0f)* XMMatrixTranslation((DotX + -400.0f), (DotY + -200.0f), 0.0f);
	cb.mWorld = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	d3dContext->PSSetShaderResources(1, 1, &Dot.billboardImageData);
	d3dContext->PSSetSamplers(0, 1, &sampler);
	//set layout to billboard vertex structure
	stride = sizeof(BillboardVertex);
	offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &Dot.billboardVertexBuffer, &stride, &offset);
	d3dContext->Draw(Dot.GetNumVerticesInBillboardMesh(), 0);


	// Display frame immediately
	swapChain->Present(0, 0);
	d3dContext->OMSetBlendState(old_state, old_factor, old_mask);
}


// Release the resources allocated to application as a result of using DirectX
void ShutdownDirectX() {
	//[20] Release skybox objects
	if (CubeMap) {
		CubeMap->Release();
	}
	if (rsCullingOn) {
		rsCullingOn->Release();
	}
	if (rsCullingOff) {
		rsCullingOff->Release();
	}
	if (DSDepthOn) {
		DSDepthOn->Release();
	}
	if (DSDepthOff) {
		DSDepthOff->Release();
	}
	if (skyboxVertexBuffer) {
		skyboxVertexBuffer->Release();
	}

	if (skyboxVertexLayout) {
		skyboxVertexLayout->Release();
	}

	if (skyboxPixelShader) {
		skyboxPixelShader->Release();
	}

	if (skyboxVertexShader) {
		skyboxVertexShader->Release();
	}
	if (TerrainIndexBuffer) {
		TerrainIndexBuffer->Release();
	}

	if (TerrainVertexBuffer) {
		TerrainVertexBuffer->Release();
	}

	if (TerrainTex)
		TerrainTex->Release();

	if (vertexShader != NULL)
		vertexShader->Release();

	if (pixelShader != NULL)
		pixelShader->Release();

	if (vertexLayout != NULL)
		vertexLayout->Release();

	if (backBuffer) {
		backBuffer->Release();
	}

	if (swapChain) {
		swapChain->Release();
	}

	if (d3dContext) {
		d3dContext->Release();
	}

	if (d3dDevice) {
		d3dDevice->Release();

		backBuffer = 0;
		swapChain = 0;
		d3dContext = 0;
		d3dDevice = 0;
	}
}

bool CreateRasterizers(HWND hMainWnd) {
	// Define a rasterizer that turns of culling and fills using solid mode
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = true;

	// Create the rasterizer
	HRESULT result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOff);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), szAppClassName, NULL);
	}

	// Now create a rasterizer that turns culling on
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_BACK;
	result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOn);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), szAppClassName, NULL);
	}

	// Create depth stencil object for disabling depth test
	// used when rendering skybox
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = false;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOff);

	// Create depth stencil object for enable depth test
	// used when rendering everything else
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOn);

	//Creating Blendstate for trnasparency
	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));

	BlendState.AlphaToCoverageEnable = true;
	BlendState.RenderTarget[0].BlendEnable = TRUE;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	d3dDevice->CreateBlendState(&BlendState, &g_pBlendState);


	return true;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLineArgs, int nInitialWinShowState) {
	// Initialise COM for DirectXTK WIC
	HRESULT hrInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Check initialised ok, if not then exit program as may get spurious errors
	// such as memory access violations if not initialised
	if (hrInit != S_OK) {
		MessageBox(NULL, L"Unable to initialise for DirectXTK WIC", szAppClassName, 0);

		return 0;
	}

	HWND hMainWnd;
	MSG msg = { 0 };
	// Find out what a WNDCLASS is and what it is used for
	WNDCLASS wndclass;
	wchar_t fps[64];
	ZeroMemory(fps, 64);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_CROSS);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;

	// Find out why we have to register the class
	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, L"Unable to register class for application", szAppClassName, 0);

		return 0;
	}

	// Find out what CreateWindows does
	hMainWnd = CreateWindow(szAppClassName,
		L"GameGraphicsProgramming - Ben Sturland",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hMainWnd) {
		MessageBox(NULL, L"Unable able to create the application's main window", szAppClassName, 0);

		return 0;
	}

	ShowWindow(hMainWnd, nInitialWinShowState);

	if (!InitialiseDX(hMainWnd, hInstance)) {
		MessageBox(NULL, L"Failed to initialise DirectX", szAppClassName, 0);

		return 0;
	}

	// Create shaders
	if (!CreateShaders(hMainWnd)) {
		// Exit program if an error occurred
		return 0;
	}

	// Create the rasterizers and depth states for the skybox rendering
	CreateRasterizers(hMainWnd);

	// Create the vertices of the triangle in DirectX
	if (!CreateMeshes(hMainWnd)) {
		// Exit program if an error occurred
		return 0;
	}

	//Create the skybox
	if (!CreateCubemapSkybox(L".\\assets\\up.jpg",
		L".\\assets\\down.jpg",
		L".\\assets\\left.jpg",
		L".\\assets\\right.jpg",
		L".\\assets\\front.jpg",
		L".\\assets\\back.jpg", 512)) {
		return 0;
	}



	SetRasterizerState(hMainWnd);

	// Slightly different message loop
	DWORD current = GetTickCount();
	frameTime = current;

	int count = 0;

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Update();

			Draw();

			count++;

			DWORD now = GetTickCount();
			if (now - current > 1000) {
				wsprintf(fps, L"FPS = %d", count);

				SetWindowText(hMainWnd, fps);

				count = 0;

				current = now;
			}

		}
	}

	ShutdownDirectX();

	return 0;
}

// Windows message procedure, this responds to user and system generated events
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		// When the user closes the window by pressing the close button
		// Tell the application to terminate by breaking the windows message loop

	case WM_KEYDOWN:
		ProcessKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		ProcessKeyUp(wParam);
		return 0;

		//[4] Write source code to respond to mouse move messages
	case WM_MOUSEMOVE:
		curMouseXPos = LOWORD(lParam);
		curMouseYPos = HIWORD(lParam);

		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}