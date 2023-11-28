#pragma once

class MyPhilpComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Gauss;
		FRDGTextureRef DisplaceHeight;
		FRDGTextureRef DisplaceX;
		FRDGTextureRef DisplaceY;
		int size;
		FVector4f WindAndSeed;
		float Time,A, G,Smooth;
	};
	
	static FRDGPassRef AddPhilpPass(const FInfoDesc& Input);
};
