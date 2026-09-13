// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.h"
#include "InventoryItemFragment_Dawnloadable.generated.h"

/**
 * 
 */
UCLASS()
class HORRORDRB5_8_API UInventoryItemFragment_Dawnloadable : public UInventoryItemFragment
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options")
	bool CanBeCanceled = true;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMesh> ItemMesh;
	
};
