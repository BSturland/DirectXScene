#include "Header.hlsli"

//Define the textures for the water
Texture2D waterdata : register(t1);
Texture2D bottomdata : register(t2);


// Sampler object
SamplerState colourSampler : register(s0);


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
	//Obtain the colour of the water based on vertex
	float4 waterColour = waterdata.Sample(colourSampler,input.texCoord);
	float4 bottomColour = bottomdata.Sample(colourSampler, input.texCoord);
	//reduce watrer alpha
	waterColour[3] *= 0.1f;
	return bottomColour * waterColour;




}


