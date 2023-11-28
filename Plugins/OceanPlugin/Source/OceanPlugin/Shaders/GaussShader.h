#pragma once

class MyGaussComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Output;
		int Size;
		float Random;
	};
	
	static FRDGPassRef AddPass(const FInfoDesc& Input);
};
