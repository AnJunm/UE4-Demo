//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Subsystems/GameInstanceSubsystem.h"
//#include "MyGameInstanceSubsystem.generated.h"
//
///**
// * 
// */
//
//UCLASS()
//class UMyGamesStatsSubsystem : public UGameInstanceSubsystem
//{
//    GENERATED_BODY()
//public:
//    TArray<MyPlayerinput> m_PlayerInput;
//    // Begin USubsystem
//    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
//    virtual void Deinitialize() override;
//    // End USubsystem
//
//    void IncrementResourceStat();
//
//    void SetPlayerInput(int PlayerIndex, MyPlayerinput Input);
//
//    void BroadcastPlayerInput();
//
//private:
//    // All my variables
//};