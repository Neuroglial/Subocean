#pragma once

class MyGenNormalComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Input;
		FRDGTextureRef Normal;
		int size;
		float SizeInWorld;
	};
	
	static FRDGPassRef AddGenNormalPass(const FInfoDesc& Input);
	
};
