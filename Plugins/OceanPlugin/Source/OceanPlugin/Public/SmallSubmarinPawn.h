// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/Pawn.h"
#include "SmallSubmarinPawn.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class USphereComponent;


UCLASS()
class OCEANPLUGIN_API ASmallSubmarinPawn : public APawn
{
	GENERATED_BODY()

public:
	/** Please add a function description */
	
public:
	ASmallSubmarinPawn();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="默认")
	TObjectPtr<USphereComponent> Sphere;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="默认")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="默认")
	TObjectPtr<UCameraComponent> Camera;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="默认")
	double OceanHeight;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="默认")
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = Input,meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = Input,meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void Move(const FInputActionValue& Value);
	UFUNCTION(BlueprintNativeEvent)
	void Look(const FInputActionValue& Value);
	
	double deltaTime;
	FVector2D InputRotation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="默认")
	float DeltaSeconds;

};
