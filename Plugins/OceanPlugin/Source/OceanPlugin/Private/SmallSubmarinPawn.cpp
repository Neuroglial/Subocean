// Fill out your copyright notice in the Description page of Project Settings.


#include "SmallSubmarinPawn.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SphereComponent.h"

// Sets default values
ASmallSubmarinPawn::ASmallSubmarinPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(RootComponent);
	Sphere->SetCollisionProfileName(FName("BlockAll"));
	Sphere->SetWorldScale3D({6,6,6});
	

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->AttachToComponent(Sphere,FAttachmentTransformRules::KeepRelativeTransform);
	Camera->SetWorldScale3D({0.166667,0.166667,0.166667});
	Camera->SetWorldLocation({1.666667,0,8.333333});

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	StaticMesh->AttachToComponent(Camera,FAttachmentTransformRules::KeepRelativeTransform);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SmallSubmarineAssert(
		TEXT("/Script/Engine.StaticMesh'/OceanPlugin/Mesh/SmallSubmarine.SmallSubmarine'"));

	if(SmallSubmarineAssert.Succeeded())
	{
		StaticMesh->SetStaticMesh(SmallSubmarineAssert.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("Movement");

	FloatingPawnMovement->MaxSpeed = 1200;
	FloatingPawnMovement->Acceleration = 800;
	FloatingPawnMovement->Deceleration = 600;
}

// Called when the game starts or when spawned
void ASmallSubmarinPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASmallSubmarinPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->deltaTime = DeltaTime;

	auto NowRotation = GetActorTransform().Rotator();
	auto DeltaInput = InputRotation/8;
	InputRotation-=DeltaInput;
	DeltaInput.X=FMath::Clamp(DeltaInput.X,-360*DeltaTime,360*DeltaTime);

	NowRotation.Yaw+=DeltaInput.X;
	NowRotation.Pitch+=DeltaInput.Y;
	
	NowRotation.Pitch = FMath::Clamp(NowRotation.Pitch,-85.0,85.0);

	SetActorRotation(NowRotation);

	if(GetActorLocation().Z>OceanHeight+35)
	{
		FloatingPawnMovement->MaxSpeed = 3600;
		FloatingPawnMovement->Acceleration = 900;
		FloatingPawnMovement->Deceleration = 0;
			
		FloatingPawnMovement->AddInputVector({0.0,0.0,-1.0});
	}
	else
	{
		FloatingPawnMovement->MaxSpeed = 1200;
		FloatingPawnMovement->Acceleration = 800;
		FloatingPawnMovement->Deceleration = 600;
	}
}

// Called to bind functionality to input
void ASmallSubmarinPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction,ETriggerEvent::Triggered,this,&ASmallSubmarinPawn::Move);
		EnhancedInput->BindAction(LookAction,ETriggerEvent::Triggered,this,&ASmallSubmarinPawn::Look);
	}
}

void ASmallSubmarinPawn::Move_Implementation(const FInputActionValue& Value)
{
	FVector move =  Value.Get<FVector>();

	if(GetActorLocation().Z>OceanHeight+35)
	{
		return;
	}

	auto right = GetActorRightVector()*move.X;
	auto forword = GetActorForwardVector()*move.Y;
	auto Up = FVector{0.0,0.0,1.0}*move.Z;
	
	FloatingPawnMovement->AddInputVector(right+forword+Up);
}

void ASmallSubmarinPawn::Look_Implementation(const FInputActionValue& Value)
{
	auto Look =  Value.Get<FVector2D>();

	float RotationSpeed = 0.25;
	
	InputRotation.X+=Look.X*RotationSpeed;
	InputRotation.Y-=Look.Y*RotationSpeed;
}
