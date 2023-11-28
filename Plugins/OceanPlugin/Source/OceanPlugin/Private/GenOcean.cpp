// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\Public\GenOcean.h"

#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RenderGraphUtils.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "ShaderCompilerCore.h"
#include "EngineDefines.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "RenderGraphResources.h"
#include "MeshPassProcessor.inl"
#include "MyBase.h"
#include "StaticMeshResources.h"

#include "OceanPlugin/Shaders/FFTShader.h"
#include "OceanPlugin/Shaders/CopyShader.h"
#include "OceanPlugin/Shaders/GenHeightShader.h"
#include "OceanPlugin/Shaders/GenNormalShader.h"
#include "OceanPlugin/Shaders/PhlipShader.h"


void FOceanInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FParams Params,
	TFunction<void()> Callback)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	int size = Params.Gauss->SizeX;
	
	const auto Gauss = RegisterExternalTexture(
		GraphBuilder,
		Params.Gauss->GetResource()->TextureRHI,
		TEXT("Gauss"));

	const auto NormalRT = RegisterExternalTexture(
		GraphBuilder,
		Params.NormalRT->GetResource()->TextureRHI,
		TEXT("NormalRT"));

	const auto DisplaceRT = RegisterExternalTexture(
		GraphBuilder,
		Params.DisplaceRT->GetResource()->TextureRHI,
		TEXT("DisplaceRT"));

	const auto NormalRDG = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({Params.NormalRT->SizeX,Params.NormalRT->SizeY},Params.NormalRT->GetFormat(),FClearValueBinding::Black,TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("NormalRDG"));
	
	const auto HeightRDG = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("HeightRDG"));

	const auto DisplaceXRDG = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("DisplaceXRDG"));
	
	const auto DisplaceYRDG = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("DisplaceXRDG"));

	const auto DisplaceRDG_Temp = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},Params.DisplaceRT->GetFormat(),FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("Temp"));

	const auto FFTH = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("FFTH"));
	

	const auto FFTX = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("FFTX"));

	const auto FFTY = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({size,size},PF_G32R32F,FClearValueBinding::Black,TexCreate_ShaderResource|TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("FFTY"));

	MyPhilpComputShader::FInfoDesc PhlipDesc{
		GraphBuilder,
		Gauss,
		HeightRDG,
		DisplaceXRDG,
		DisplaceYRDG,
		size,
		Params.WindAndSeed,
		Params.Time,
		Params.A,
		Params.G,
		Params.Smooth
	};
	auto PhilpPass = MyPhilpComputShader::AddPhilpPass(PhlipDesc);
	
	MyFFTComputShader::FInfoDesc HFFTinfo{
		GraphBuilder,HeightRDG,FFTH,DisplaceRDG_Temp,size,false};
	auto HFFTPass = MyFFTComputShader::AddFFTPass(HFFTinfo);
	GraphBuilder.AddPassDependency(PhilpPass,HFFTPass);

	MyFFTComputShader::FInfoDesc XFFTinfo{
		GraphBuilder,DisplaceXRDG,FFTX,DisplaceRDG_Temp,size,false};
	auto XFFTPass =  MyFFTComputShader::AddFFTPass(XFFTinfo);
	GraphBuilder.AddPassDependency(HFFTPass,XFFTPass);
	
	MyFFTComputShader::FInfoDesc YFFTinfo{
		GraphBuilder,DisplaceYRDG,FFTY,DisplaceRDG_Temp,size,false};
	auto YFFTPass =  MyFFTComputShader::AddFFTPass(YFFTinfo);
	GraphBuilder.AddPassDependency(XFFTPass,YFFTPass);

	MyCopyComputShader::FInfoDesc CopyXInfo{
		GraphBuilder,FIntVector4(1,0,0,0),FFTX,DisplaceRDG_Temp,size};
	auto CopyPassX = MyCopyComputShader::AddCopyPass(CopyXInfo);
	GraphBuilder.AddPassDependency(YFFTPass,CopyPassX);

	MyCopyComputShader::FInfoDesc CopyYInfo{
		GraphBuilder,FIntVector4(0,1,0,0),FFTY,DisplaceRDG_Temp,size};
	auto CopyPassY = MyCopyComputShader::AddCopyPass(CopyYInfo);
	GraphBuilder.AddPassDependency(CopyPassX,CopyPassY);

	MyCopyComputShader::FInfoDesc CopyZInfo{
		GraphBuilder,FIntVector4(0,0,1,0),FFTH,DisplaceRDG_Temp,size,FVector4f(1.0f,1.0f,-1.0f,1.0f)};
	auto CopyPassZ = MyCopyComputShader::AddCopyPass(CopyZInfo);
	GraphBuilder.AddPassDependency(CopyPassY,CopyPassZ);
	
	MyGenNormalComputShader::FInfoDesc GenNormalInfo{
		GraphBuilder,DisplaceRDG_Temp,NormalRDG,size,Params.SizeInWorld};
	auto NormalPass = MyGenNormalComputShader::AddGenNormalPass(GenNormalInfo);
	GraphBuilder.AddPassDependency(CopyPassZ,NormalPass);

	MyGenHeightComputShader::FInfoDesc GenHeightInfo{
		GraphBuilder,DisplaceRDG_Temp,NormalRDG,size,Params.SizeInWorld};
	auto HeightPass = MyGenHeightComputShader::AddPass(GenHeightInfo);
	GraphBuilder.AddPassDependency(NormalPass,HeightPass);

	AddCopyTexturePass(GraphBuilder,DisplaceRDG_Temp,DisplaceRT);
	AddCopyTexturePass(GraphBuilder,NormalRDG,NormalRT);
	
	GraphBuilder.Execute();
	OceanTool::CallBack(Callback);
}
