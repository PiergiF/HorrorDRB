#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartPcScreenWidgetDRB.generated.h"


// Forward declarations
class UWidgetSwitcher;
class UButton;
class UTextBlock;
class UCheckBox;
//class UEosLoginWidgetDRB;

/**
 *
 */
UCLASS()
class HORRORDRB5_8_API UStartPcScreenWidgetDRB : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// --- COMPONENTI PRINCIPALI ---
	
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UWidgetSwitcher* ScreenSwitcher; // Gestisce quale schermata mostrare

	// --- PANNELLO 0: PRE-LOGIN ---
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* Panel_PreLogin;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_LoginEpic;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UCheckBox* CheckBox_RememberMe;

	///OLD PER BROWSER INTERNO AL GIOCO
	// --- PANNELLO 1: BROWSER LOGIN (Il tuo WebBrowser) ---
	//UPROPERTY(meta = (BindWidget))
	//UEosLoginWidgetDRB* Widget_WebBrowser;
	///

	// --- PANNELLO 1: ATTESA LOGIN BROWSER ---
	
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* Panel_WaitingLogin;

	// Bottone per annullare l'attesa se il giocatore cambia idea
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_CancelLogin;

	// --- PANNELLO 2: POST-LOGIN (LOBBY) ---
	
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* Panel_PostLogin;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Nickname;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Logout;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_CreateLobby;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_SearchLobby;

	// --- PANNELLO 3: CREA LOBBY (NUOVO) ---
	
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* Panel_CreateLobby;

	//UPROPERTY(meta = (BindWidget))
	//class UEditableTextBox* TextBox_LobbyName;
	//
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UEditableText* TextBox_LobbyName;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ConfirmCreate;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_BackFromCreate;

	// --- PANNELLO 4: CERCA LOBBY (NUOVO) ---
	
	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* Panel_FindLobby;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UScrollBox* ScrollBox_LobbyList;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UPROPERTY(BlueprintReadWrite,  meta = (BindWidgetOptional))
	UButton* Btn_RefreshList;

	//UPROPERTY(meta = (BindWidget))
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_BackFromFind;


	// --- FUNZIONI DI BINDING ---
	/* Funzioni per il login*/
	UFUNCTION()
	void OnLoginEpicClicked();

	UFUNCTION()
	void OnCancelLoginClicked();

	UFUNCTION()
	void OnLogoutClicked();

	UFUNCTION()
	void HandleLoginComplete(bool bWasSuccessful);

	/* Funzioni per la Lobby*/
	UFUNCTION()
	void OnCreateLobbyClicked();

	UFUNCTION()
	void HandleLobbyCreated(bool bWasSuccessful);

	UFUNCTION()
	void OnSearchLobbyClicked(); // Va al pannello 4

	UFUNCTION()
	void OnConfirmCreateClicked(); // Avvia la creazione vera e propria

	UFUNCTION()
	void OnRefreshListClicked(); // Avvia la ricerca sui server

	UFUNCTION()
	void OnBackToHubClicked(); // Tasto indietro universale per tornare al pannello 2


	/** Abilita questo widget a ricevere il focus della tastiera */
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }

	/** Cattura la pressione dei tasti quando il widget è a schermo */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Cattura il focus quando il mouse passa sullo schermo 3D */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Intercetta i click del mouse per evitare la perdita di focus */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};