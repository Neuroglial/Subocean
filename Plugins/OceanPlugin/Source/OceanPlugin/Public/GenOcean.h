// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"

#include "GenOcean.generated.h"

class FOceanInterface
{
public:
	struct FParams
	{
		FVector4f WindAndSeed;
		float A,G,Time,Smooth,SizeInWorld;
		UTextureRenderTarget2D* Gauss;
		UTextureRenderTarget2D* DisplaceRT;
		UTextureRenderTarget2D* NormalRT;
	};
	
private:
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FParams Params,
		TFunction<void()> Callback
		);
	
	static void DispatchGameThread(
		FParams Params,
		TFunction<void()> Callback)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params, Callback](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params, Callback);
		});
	}
	
public:

	static void Dispatch(
		const FParams& Params,
		const TFunction<void()>& Callback)
	{
		const int size = Params.Gauss->SizeX;

		if(Params.DisplaceRT->SizeY!=size)
			Params.DisplaceRT->ResizeTarget(size,size);
		
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, Callback);
		}else{
			DispatchGameThread(Params, Callback);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPhlipSpectrumCompeleted);
UCLASS()
class UGenOceanAsync: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FPhlipSpectrumCompeleted Completed;

	FOceanInterface::FParams Params;
	
	virtual  void Activate() override
	{
		FOceanInterface::Dispatch(Params,[this]{
			Completed.Broadcast();
		});
	}
	
	UFUNCTION(BlueprintCallable, meta = (Category = " ComputeShader",BlueprintInternalUseOnly = "true",WorldContext = "WorldContextObject",DisplayName = "Execute Gen FFT Ocean"))
	static UGenOceanAsync* Execute(UObject* WorldContextObject,
		UTextureRenderTarget2D* Gauss,
		UTextureRenderTarget2D* DisplaceRT,
		UTextureRenderTarget2D* NormalRT,
		FVector4f WindAndSeed,
		float Time,float A = 0.2f,float G = 9.8f,float Smooth = 1.0f,float SizeInWorld = 100.0f)
	{
		auto* Action = NewObject<UGenOceanAsync>();
		Action->RegisterWithGameInstance(WorldContextObject);
		Action->Params = {
			WindAndSeed,
			A,G,Time,Smooth,SizeInWorld,
			Gauss,
			DisplaceRT,
			NormalRT};
		
		Action->RegisterWithGameInstance(WorldContextObject);
		return Action;
	}
};