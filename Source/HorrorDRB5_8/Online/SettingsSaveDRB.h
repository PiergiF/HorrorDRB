#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsSaveDRB.generated.h"

/**
 * 
 */
UCLASS()
class HORRORDRB5_8_API USettingsSaveDRB : public USaveGame
{
	GENERATED_BODY()
	
public:
	// Memorizza se il giocatore vuole l'autologin
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EOS")
	bool bAutoLoginEnabled = false;
};
