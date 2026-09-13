// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeBaseDRB.generated.h"

/**
 * 
 */
UCLASS()
class HORRORDRB5_8_API AGameModeBaseDRB : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Constructor
	AGameModeBaseDRB();

protected:
	virtual void BeginPlay() override;
	
};
