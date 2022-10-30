// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
//enum PlayerInputEnum :int 
//{
//	Forward = 0,
//	Back = 1,
//	Right = 2,
//	Left = 3,
//};


namespace MyEnum
{
	enum class EPlayerInputEnum :uint8 {
		Forward UMETA(DisplayName = "DOWN"),
		Back UMETA(DisplayName = "LEFT"),
		Right UMETA(DisplayName = "UP"),
		Left
	};
}