// Copyright Epic Games, Inc. All Rights Reserved.
#include "ThirdPersonMPCharacter.h"
#include "ThirdPersonMPProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AThirdPersonMPCharacter

AThirdPersonMPCharacter::AThirdPersonMPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);



	//bUsingMotionControllers = true;

	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

//////////////////////////////////////////////////////////////////////////
// Replicated Properties

void AThirdPersonMPCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AThirdPersonMPCharacter, CurrentHealth);
}

void AThirdPersonMPCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AThirdPersonMPCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AThirdPersonMPCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}


void AThirdPersonMPCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

void AThirdPersonMPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	//PrimaryActorTick.bCanEverTick = true;
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh1P->SetHiddenInGame(false, true);
}

//////////////////////////////////////////////////////////////////////////
// Input
void AThirdPersonMPCharacter::ReceiveTick(float DeltaSeconds)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("Tick has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

}

void AThirdPersonMPCharacter::Tick(float DeltaSeconds)
{

	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("Tick has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		m_Input.UID = GetPlayerId();
		m_Input.InputValue = 0;	
		Server_SendInput(m_Input);
	}
}

void AThirdPersonMPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AThirdPersonMPCharacter::OnFire);


	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AThirdPersonMPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AThirdPersonMPCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AThirdPersonMPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AThirdPersonMPCharacter::LookUpAtRate);
}

void AThirdPersonMPCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AThirdPersonMPProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

			Server_OnFire(SpawnLocation, SpawnRotation);
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

bool AThirdPersonMPCharacter::Server_OnFire_Validate(FVector Location, FRotator Rotation)
{
	return GetLocalRole() == ROLE_Authority;
}

void AThirdPersonMPCharacter::Server_OnFire_Implementation(FVector Location, FRotator Rotation)
{
	UE_LOG(LogTemp, Warning, TEXT("Server"));
	FString healthMessage = FString::Printf(TEXT("Server"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
}

bool AThirdPersonMPCharacter::Server_SendInput_Validate(FMyPlayerInput input)
{
	return (GetLocalRole() == ROLE_Authority);
}

void AThirdPersonMPCharacter::Server_SendInput_Implementation(FMyPlayerInput input)
{
	ClientInput.Add(input);
	if (ClientInput.Num() == 2)
	{
		RPC_BroadcastInput(ClientInput);
		ClientInput.Empty();
	}
}

bool AThirdPersonMPCharacter::RPC_BroadcastInput_Validate(const TArray<FMyPlayerInput>& input)
{
	return true;
}

void AThirdPersonMPCharacter::RPC_BroadcastInput_Implementation(const TArray<FMyPlayerInput>& input)
{
	//移动PlayId不等于参数的玩家位置
	for (const FMyPlayerInput& _ : input)
	{
		MoveOtherPlayers(_);
	}
}

#pragma optimize( "", off) 
void AThirdPersonMPCharacter::MoveOtherPlayers(const FMyPlayerInput& input)
{
	TSubclassOf<AThirdPersonMPCharacter> ClassToFind; // Needs to be populated somehow (e.g. by exposing to blueprints as uproperty and setting it there
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AThirdPersonMPCharacter::StaticClass(), FoundActors);
	for (AActor* _actor : FoundActors)
	{
		AThirdPersonMPCharacter* actor = Cast<AThirdPersonMPCharacter>(_actor);
		if (actor)
		{
			//curUID是当前actor所属玩家的id, 而input.UID是服务器同步给所有玩家的操作数据，GetPlayerId是当前玩家的uid
			int curUID = GetUId(actor);
			//如果UID相等则不做操作
			if (input.UID == GetPlayerId())
			{
				continue;
			}
			//curUID是当前角色的UID,只要和输入的一致，就去移动
			if (input.UID == curUID)
			{
				FVector dir = (input.InputDir == EPlayerInputEnum::Forward || input.InputDir == EPlayerInputEnum::Back) ? GetActorForwardVector() : GetActorRightVector();
				actor->AddMovementInput(dir, input.InputValue);
			}
			//=Todo:获取Actor的PlayerId,只要与参数相同，就执行运动
		}
	}
};

int AThirdPersonMPCharacter::GetUId(AActor* actor)
{
	APawn* Pawn = Cast<APawn>(actor);
	AController* PC = Pawn->GetController();
	int res = PC->PlayerState->GetPlayerId();
	return res;
}

int AThirdPersonMPCharacter::GetPlayerId()
{
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AController* controller = Iterator->Get();
		APlayerState* PC = controller->PlayerState;
		int playerId = PC->GetPlayerId();
		return playerId;
	}
	return 1;
}
#pragma optimize( "", on) 
void AThirdPersonMPCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		EPlayerInputEnum dir = Value < 0 ? EPlayerInputEnum(EPlayerInputEnum::Back) : EPlayerInputEnum(EPlayerInputEnum::Forward);
		m_Input.InputValue = Value;
		m_Input.InputDir = dir;
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AThirdPersonMPCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		EPlayerInputEnum dir = Value < 0 ? EPlayerInputEnum(EPlayerInputEnum::Left) : EPlayerInputEnum(EPlayerInputEnum::Right);
		m_Input.InputValue = Value;
		m_Input.InputDir = dir;
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AThirdPersonMPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AThirdPersonMPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}