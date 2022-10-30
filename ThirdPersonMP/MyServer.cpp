// Fill out your copyright notice in the Description page of Project Settings.


#include "MyServer.h"

// Sets default values
AMyServer::AMyServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyServer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AMyServer::BroadcastInputToClient()
{
	

}