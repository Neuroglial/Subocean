// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\Public\GenGaussian.h"

#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "ShaderCompilerCore.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"
#include "EngineDefines.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "RenderGraphResources.h"
#include "MeshPassProcessor.inl"
#include "StaticMeshResources.h"
#include "../Shaders/GaussShader.h"
#include "MyBase.h"

void FGaussianInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FParams Params, TFunction<void()> Callback)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	
	const auto OutTex = GraphBuilder.CreateTexture(
		FRDGTextureDesc::Create2D({Params.Size,Params.Size},Params.Output->GetFormat(),FClearValueBinding::Black,TexCreate_UAV|TexCreate_RenderTargetable),
		TEXT("Output"));

	const auto OutTarget = RegisterExternalTexture(
		GraphBuilder,Params.Output->GetResource()->TextureRHI,TEXT("OutTarget"));

	MyGaussComputShader::FInfoDesc GaussInfo{
		GraphBuilder,
		OutTex,
		Params.Size,
		Params.Random
	};
	
	MyGaussComputShader::AddPass(GaussInfo);
	AddCopyTexturePass(GraphBuilder, OutTex,OutTarget);

	GraphBuilder.Execute();
	OceanTool::CallBack(Callback);
}
