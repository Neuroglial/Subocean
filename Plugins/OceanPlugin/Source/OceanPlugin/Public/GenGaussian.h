// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"

#include "GenGaussian.generated.h"

class FGaussianInterface
{
public:
	struct FParams
	{
		UTextureRenderTarget2D* Output;
		int Size;
		float Random;
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
		if(Params.Output->SizeX!=Params.Size||Params.Output->SizeY!=Params.Size)
			Params.Output->ResizeTarget(Params.Size,Params.Size);
		
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, Callback);
		}else{
			DispatchGameThread(Params, Callback);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenGuassCompeleted);
UCLASS()
class UGenGaussAsync: public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FGenGuassCompeleted Completed;

	FGaussianInterface::FParams Params;
	
	virtual void Activate() override
	{
		FGaussianInterface::Dispatch(Params,[this]{
			Completed.Broadcast();
		});
	}
	
	UFUNCTION(BlueprintCallable, meta = (Category = " ComputeShader",BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Execute Gen Gauss"))
	static UGenGaussAsync* Execute(UObject* WorldContextObject,UTextureRenderTarget2D* Output,int Size, float Random)
	{
		auto* Action = NewObject<UGenGaussAsync>();
		
		Action->Params = {
			Output,Size,Random
		};

		Action->RegisterWithGameInstance(WorldContextObject);
		return Action;
	}
	
};