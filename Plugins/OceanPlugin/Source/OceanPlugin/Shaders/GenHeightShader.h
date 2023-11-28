#pragma once

class MyGenHeightComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Input;
		FRDGTextureRef Height;
		int size;
		float SizeInWorld;
	};
	
	static FRDGPassRef AddPass(const FInfoDesc& Input);
	
};

