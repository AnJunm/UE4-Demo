// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyServer.generated.h"

UCLASS()
class THIRDPERSONMP_API AMyServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyServer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//Get the input comd.
	
	//void GetInput();
	//BroadCast the comd.

	//UFUNCTION(Client)
	void BroadcastInputToClient();
};
