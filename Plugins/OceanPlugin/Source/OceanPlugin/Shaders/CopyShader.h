#pragma once

class MyCopyComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FIntVector4 Select;
		FRDGTextureRef Input;
		FRDGTextureRef Output;
		int size;
		FVector4f Multi = FVector4f(1.0,1.0,1.0,1.0);
	};
	
	static FRDGPassRef AddCopyPass(const FInfoDesc& Input);
	
};
