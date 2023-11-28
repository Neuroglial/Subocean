// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OceanCreator.generated.h"

UCLASS()
class OCEANPLUGIN_API AOceanCreator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOceanCreator();

public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="海洋材质参数")
	TObjectPtr<UMaterialParameterCollection> OceanParameters;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="高斯分布贴图")
	TObjectPtr<UTextureRenderTarget2D> GaussRT;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="置换贴图")
	TObjectPtr<UTextureRenderTarget2D> DisplaceRT;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="法线贴图")
	TObjectPtr<UTextureRenderTarget2D> NormalRT;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="贴图大小")
	int TexTRSize = 256;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="随机种子")
	float RandomSeed = 12.0f;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="风向和大小")
	FVector4f WindAndSeed{12.0,0.0f,0.0f,0.0f};
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="浪的大小")
	float A = 0.015f;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="重力")
	float G = 20.0f;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认",DisplayName="波浪平滑度")
	float Smooth = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	float SizeInWorld = 2560.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
