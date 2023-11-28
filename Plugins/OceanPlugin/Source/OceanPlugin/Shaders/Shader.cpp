#include "CopyShader.h"
#include "GenNormalShader.h"
#include "PhlipShader.h"
#include "FFTShader.h"
#include "GaussShader.h"
#include "GenHeightShader.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "MyBase.h"
#include "RenderGraphUtils.h"

class FCopyCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCopyCS);
	SHADER_USE_PARAMETER_STRUCT(FCopyCS,FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(FIntVector4,Select)
		SHADER_PARAMETER(FVector4f,Multi)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D,Input)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>,Output)
	END_SHADER_PARAMETER_STRUCT();
	
};
IMPLEMENT_GLOBAL_SHADER(FCopyCS,"/Shaders/FCopyCS.usf","MainCS",SF_Compute);

FRDGPassRef MyCopyComputShader::AddCopyPass(const FInfoDesc& Input)
{
	TShaderMapRef<FCopyCS> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FCopyCS::FParameters* pms = Input.GraphBuilder.AllocParameters<FCopyCS::FParameters>();

	pms->Select = Input.Select;
	pms->Input = Input.GraphBuilder.CreateSRV(Input.Input);
	pms->Output = Input.GraphBuilder.CreateUAV(Input.Output);
	pms->Multi = Input.Multi;

	FIntVector Group = {Input.size/32,Input.size/32,1};
	
	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteCopyPass"),pms,
		ERDGPassFlags::AsyncCompute,
		[Compute,pms,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*pms,Group);
		});

	return Pass;
}

BEGIN_SHADER_PARAMETER_STRUCT(FFFTParameters,)
		SHADER_PARAMETER(FIntVector3,StepNUM_Inverse_LineX)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<int>,IndexBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float2>,InputRT)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>,OutputRT)
END_SHADER_PARAMETER_STRUCT();

template <int Size>
class FFFTCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFFTCS);
	SHADER_USE_PARAMETER_STRUCT(FFFTCS,FGlobalShader);
	using FParameters = FFFTParameters;
	
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM"),Size );
	}
};

IMPLEMENT_GLOBAL_SHADER(FFFTCS<16>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<32>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<64>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<128>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<256>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<512>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FFFTCS<1024>,"/Shaders/FFTCS.usf","MainCS",SF_Compute);

class ReverseBit
{
private:
	std::vector<int> Bits;
public:
	ReverseBit(int size)
	{
		Bits.resize(size);
		OceanTool::BitReverse(Bits);
	}

	inline void* Get()
	{
		return Bits.data();
	}
};

template <int Size>
FRDGPassRef MyFFTComputShader::FFTPass(const FInfoDesc& Input)
{
	FSingleInfoDesc Pass1Desc{
		Input.GraphBuilder,
		Input.Input,
		Input.Temp,
		Size,
		Input.Inverse,
		true
	};

	FSingleInfoDesc Pass2Desc{
		Input.GraphBuilder,
		Input.Temp,
		Input.Output,
		Size,
		Input.Inverse,
		false
	};

	auto Pass1 = FFTPassSingleLine<Size>(Pass1Desc);
	auto Pass2 = FFTPassSingleLine<Size>(Pass2Desc);
	
	Input.GraphBuilder.AddPassDependency(Pass1,Pass2);

	return Pass2;
}

template <int Size>
FRDGPassRef MyFFTComputShader::FFTPassSingleLine(const FSingleInfoDesc& Input)
{
	TShaderMapRef<FFFTCS<Size>> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	
	FFFTParameters* ParametersPass = Input.GraphBuilder.AllocParameters<FFFTParameters>();

	static ReverseBit ReBitBuffer(Size);
	
	auto IndexUploadBuffer = CreateUploadBuffer(Input.GraphBuilder,
		TEXT("IndexBuffer"),
		sizeof(int),
		Size,
		ReBitBuffer.Get(),
		Size*sizeof(int));

	auto IndexSRV = Input.GraphBuilder.CreateSRV(IndexUploadBuffer,PF_R32_SINT);
	
	ParametersPass->StepNUM_Inverse_LineX =  FIntVector3(
		static_cast<int>(log2(Size)),
		Input.Inverse?-1:1,
		Input.LineX);
	
	ParametersPass->IndexBuffer = IndexSRV;
	ParametersPass->InputRT = Input.GraphBuilder.CreateSRV(Input.Input);
	ParametersPass->OutputRT = Input.GraphBuilder.CreateUAV(Input.Output);

	FIntVector Group = {1,Size,1};

	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteFFTPass2"),ParametersPass,
		ERDGPassFlags::AsyncCompute,
		[Compute,ParametersPass,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*ParametersPass,Group);
		});

	return Pass;
}

FRDGPassRef MyFFTComputShader::AddFFTPass(const FInfoDesc& Input)
{
	switch (Input.Size)
	{
	case 16:
		return FFTPass<16>(Input);
	case 32:
		return FFTPass<32>(Input);
	case 64:
		return FFTPass<64>(Input);
	case 128:
		return FFTPass<128>(Input);
	case 256:
		return FFTPass<256>(Input);
	case 512:
		return FFTPass<512>(Input);
	case 1024:
		return FFTPass<1024>(Input);
	default:
		return nullptr;
	}
}

FRDGPassRef MyFFTComputShader::AddFFTPassSingleLine(const FSingleInfoDesc& Input)
{
	switch (Input.Size)
	{
	case 16:
		return FFTPassSingleLine<16>(Input);
	case 32:
		return FFTPassSingleLine<32>(Input);
	case 64:
		return FFTPassSingleLine<64>(Input);
	case 128:
		return FFTPassSingleLine<128>(Input);
	case 256:
		return FFTPassSingleLine<256>(Input);
	case 512:
		return FFTPassSingleLine<512>(Input);
	case 1024:
		return FFTPassSingleLine<1024>(Input);
	default:
		return nullptr;
	}
}

class FGenNormalCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FGenNormalCS);
	SHADER_USE_PARAMETER_STRUCT(FGenNormalCS,FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(float,SizeInWorld)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>,Input)
		SHADER_PARAMETER_SAMPLER(SamplerState,InputSamper)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>,Normal)
	END_SHADER_PARAMETER_STRUCT();
	
};
IMPLEMENT_GLOBAL_SHADER(FGenNormalCS,"/Shaders/GenNormalCS.usf","MainCS",SF_Compute);

FRDGPassRef MyGenNormalComputShader::AddGenNormalPass(const FInfoDesc& Input)
{
	TShaderMapRef<FGenNormalCS> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FGenNormalCS::FParameters* pms = Input.GraphBuilder.AllocParameters<FGenNormalCS::FParameters>();

	FSamplerStateInitializerRHI SamplerInit;
	SamplerInit.Filter = SF_Trilinear;
	SamplerInit.AddressU = AM_Wrap;
	SamplerInit.AddressV = AM_Wrap;
	SamplerInit.AddressW = AM_Wrap;

	pms->SizeInWorld = Input.SizeInWorld;
	pms->Input = Input.GraphBuilder.CreateSRV(Input.Input);
	pms->Normal = Input.GraphBuilder.CreateUAV(Input.Normal);
	pms->InputSamper = RHICreateSamplerState(SamplerInit);

	FIntVector Group = {Input.size/32,Input.size/32,1};
	
	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteNormalPass"),pms,
		ERDGPassFlags::AsyncCompute,
		[Compute,pms,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*pms,Group);
		});

	return Pass;
}

class FPhlipSpectrumCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FPhlipSpectrumCS);
	SHADER_USE_PARAMETER_STRUCT(FPhlipSpectrumCS,FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(FVector4f, WindAndSeed)
		SHADER_PARAMETER(float, A)
		SHADER_PARAMETER(float, G)
		SHADER_PARAMETER(float, Time)
		SHADER_PARAMETER(float, Smooth)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>,GaussianRandomRT)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>,HeightSpectrumRT)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>,DisplaceXSpectrumRT)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>,DisplaceYSpectrumRT)
	END_SHADER_PARAMETER_STRUCT();
	
};
IMPLEMENT_GLOBAL_SHADER(FPhlipSpectrumCS,"/Shaders/PhlipSpectrumCS.usf","MainCS",SF_Compute);

FRDGPassRef MyPhilpComputShader::AddPhilpPass(const FInfoDesc& Input)
{
	TShaderMapRef<FPhlipSpectrumCS> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FPhlipSpectrumCS::FParameters* pms = Input.GraphBuilder.AllocParameters<FPhlipSpectrumCS::FParameters>();
	
	pms->WindAndSeed = Input.WindAndSeed;
	pms->A = Input.A;
	pms->G = Input.G,
	pms->Time = Input.Time;
	pms->Smooth = Input.Smooth;
	pms->GaussianRandomRT = Input.GraphBuilder.CreateSRV(Input.Gauss);
	pms->HeightSpectrumRT = Input.GraphBuilder.CreateUAV(Input.DisplaceHeight);
	pms->DisplaceXSpectrumRT = Input.GraphBuilder.CreateUAV(Input.DisplaceX);
	pms->DisplaceYSpectrumRT = Input.GraphBuilder.CreateUAV(Input.DisplaceY);

	FIntVector Group = {Input.size/32,Input.size/32,1};
	
	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteGenGuass"),pms,
		ERDGPassFlags::AsyncCompute,
		[Compute,pms,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*pms,Group);
		});

	return Pass;
}

class FGaussCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FGaussCS);
	SHADER_USE_PARAMETER_STRUCT(FGaussCS,FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(float,Random)
		SHADER_PARAMETER(FVector2f,Size)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>,Output)
	END_SHADER_PARAMETER_STRUCT();
	
};
IMPLEMENT_GLOBAL_SHADER(FGaussCS,"/Shaders/GenGaussCS.usf","MainCS",SF_Compute);

FRDGPassRef MyGaussComputShader::AddPass(const FInfoDesc& Input)
{
	TShaderMapRef<FGaussCS> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FGaussCS::FParameters* pms = Input.GraphBuilder.AllocParameters<FGaussCS::FParameters>();
	
	pms->Random = Input.Random;
	pms->Size = {static_cast<float>(Input.Size),static_cast<float>(Input.Size)};
	pms->Output = Input.GraphBuilder.CreateUAV(Input.Output);

	FIntVector Group = {Input.Size/32,Input.Size/32,1};

	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteGenGuass"),pms,
		ERDGPassFlags::AsyncCompute,
		[Compute,pms,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*pms,Group);
		});

	return Pass;
}

class FGenHeightCS:public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FGenHeightCS);
	SHADER_USE_PARAMETER_STRUCT(FGenHeightCS,FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(float,SizeInWorld)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float3>,Input)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>,Height)
	END_SHADER_PARAMETER_STRUCT();
	
};
IMPLEMENT_GLOBAL_SHADER(FGenHeightCS,"/Shaders/GenHeightCS.usf","MainCS",SF_Compute);

FRDGPassRef MyGenHeightComputShader::AddPass(const FInfoDesc& Input)
{
	TShaderMapRef<FGenHeightCS> Compute(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FGenHeightCS::FParameters* pms = Input.GraphBuilder.AllocParameters<FGenHeightCS::FParameters>();

	pms->SizeInWorld = Input.SizeInWorld;
	pms->Input = Input.GraphBuilder.CreateSRV(Input.Input);
	pms->Height = Input.GraphBuilder.CreateUAV(Input.Height);

	FIntVector Group = {Input.size/32,Input.size/32,1};
	
	auto Pass = Input.GraphBuilder.AddPass(
		RDG_EVENT_NAME("ExecuteHeightPass"),pms,
		ERDGPassFlags::AsyncCompute,
		[Compute,pms,Group](FRHICommandListImmediate& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList,Compute,*pms,Group);
		});

	return Pass;
}