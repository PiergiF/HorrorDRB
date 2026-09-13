// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.h"
#include "HorrorDRB5_8/InventorySystem/Items/ItemDefinition.h"
#include "InventoryItemFragment_Stackable.generated.h"

/**
 * 
 */
UCLASS()
class HORRORDRB5_8_API UInventoryItemFragment_Stackable : public UInventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options")
	int32 MaxStackCount;
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "HelperF unctions")
	static int32 GetItemMaxStackCount(const TSubclassOf<UItemDefinition> ItemDefinition);
};

