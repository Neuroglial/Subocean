// Fill out your copyright notice in the Description page of Project Settings.


#include "OceanCreator.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "..\Public\GenGaussian.h"
#include "GenOcean.h"
#include "RenderGraphUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Tests/AutomationCommon.h"

// Sets default values
AOceanCreator::AOceanCreator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    const ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> GaussRTFinder
		(TEXT("/Script/Engine.TextureRenderTarget2D'/OceanPlugin/RenderTarget/GaussRT.GaussRT'"));
	if(GaussRTFinder.Succeeded())
		GaussRT = GaussRTFinder.Object;

    const ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> DisplaceRTFinder
		(TEXT("/Script/Engine.TextureRenderTarget2D'/OceanPlugin/RenderTarget/DisplaceRT.DisplaceRT'"));
	if(GaussRTFinder.Succeeded())
		DisplaceRT = DisplaceRTFinder.Object;

    const ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> NormalRTFinder
		(TEXT("/Script/Engine.TextureRenderTarget2D'/OceanPlugin/RenderTarget/NormalRT.NormalRT'"));
	if(GaussRTFinder.Succeeded())
		NormalRT = NormalRTFinder.Object;

    const ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> OceanParametersFinder
		(TEXT("/Script/Engine.MaterialParameterCollection'/OceanPlugin/OceanParameter.OceanParameter'"));
	if(GaussRTFinder.Succeeded())
		OceanParameters = OceanParametersFinder.Object;

}

// Called when the game starts or when spawned
void AOceanCreator::BeginPlay()
{
	Super::BeginPlay();
	
	if(GaussRT)
		GaussRT->ResizeTarget(TexTRSize,TexTRSize);
	if(DisplaceRT)
		DisplaceRT->ResizeTarget(TexTRSize,TexTRSize);
	if(NormalRT)
		NormalRT->ResizeTarget(TexTRSize,TexTRSize);

	FGaussianInterface::FParams GaussParmas;
	GaussParmas.Output = GaussRT;
	GaussParmas.Size = TexTRSize;
	GaussParmas.Random = RandomSeed;

	FGaussianInterface::Dispatch(GaussParmas,[]
	{
		GEngine->AddOnScreenDebugMessage(-1,3.0f,FColor::Blue,TEXT("成功创建高斯贴图"));	
	});

	bool GetSizeInWorld = false;
	auto* params = GetWorld()->GetParameterCollectionInstance(OceanParameters);

	params->GetScalarParameterValue(FName("SizeInWorld"),SizeInWorld);
	GEngine->AddOnScreenDebugMessage(-1,3.0f,FColor::Blue,FString::Format(TEXT("海面平铺距离：{0}"),{SizeInWorld}));	
}

// Called every frame
void AOceanCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static float Time = 0.0;
	Time+=DeltaTime;
	
	FOceanInterface::FParams OceanParams{
		WindAndSeed,A,G,Time,Smooth,SizeInWorld,GaussRT,DisplaceRT,NormalRT
	};

	FOceanInterface::Dispatch(OceanParams,[]{});
}

