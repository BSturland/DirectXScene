#include "Header.hlsli"

// Image data and sampler
Texture2D imageData : register(t1);
SamplerState colourSampler : register(s0);

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
	return imageData.Sample(colourSampler, input.texCoord);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}