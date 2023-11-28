#pragma once

class MyFFTComputShader
{
public:
	struct FInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Input;
		FRDGTextureRef Output;
		FRDGTextureRef Temp;
		int Size;
		bool Inverse;
	};

	struct FSingleInfoDesc
	{
		FRDGBuilder& GraphBuilder;
		FRDGTextureRef Input;
		FRDGTextureRef Output;
		int Size;
		bool Inverse;
		bool LineX;
	};
	
private:
	template<int Size>
	static FRDGPassRef FFTPass(const FInfoDesc& Input);

	template<int Size>
	static FRDGPassRef FFTPassSingleLine(const FSingleInfoDesc& Input);

public:
	static FRDGPassRef AddFFTPass(const FInfoDesc& Input);
	static FRDGPassRef AddFFTPassSingleLine(const FSingleInfoDesc& Input);
};
