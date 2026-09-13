// Fill out your copyright notice in the Description page of Project Settings.


#include "StartPcScreenWidgetDRB.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "EosGameInstanceSubsystemDRB.h"
#include "SettingsSaveDRB.h"
#include "Components/EditableText.h"
#include "Kismet/GameplayStatics.h"

/*
void UStartPcScreenWidgetDRB::NativeConstruct()
{
	Super::NativeConstruct();

	// Controllo del SaveGame per l'Auto-Login
	if (USettingsSaveDRB* SaveSlot = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0)))
	{
		if (SaveSlot->bAutoLoginEnabled)
		{
			// Mettiamo il monitor in stato di "Attesa" visiva
			if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(1);

			// Avviamo il login in background. Passando "true", il Subsystem userà PersistentAuth!
			if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
			{
				EOS_Subsystem->LoginWithEpic(GetOwningPlayer(), true);
			}
			// Interrompiamo l'esecuzione per non mostrare il pannello di pre-login
			return;
		}
	}

	// Se il file di salvataggio non esiste o l'autologin è disabilitato,
	// Inizializza lo stato: Mostra il pannello di Pre-Login
	if (ScreenSwitcher)
	{
		ScreenSwitcher->SetActiveWidgetIndex(0);
	}

	// Binda i click dei bottoni
	if (Btn_LoginEpic)
	{
		Btn_LoginEpic->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnLoginEpicClicked);
	}
	if (Btn_CancelLogin)
	{
		Btn_CancelLogin->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnCancelLoginClicked);
	}
	if (Btn_Logout)
	{
		Btn_Logout->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnLogoutClicked);
	}

	// Binda l'evento del subsystem per sapere quando il login finisce
	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		EOS_Subsystem->OnEpicLoginComplete.AddDynamic(this, &UStartPcScreenWidgetDRB::HandleLoginComplete);
	}
}
*/

void UStartPcScreenWidgetDRB::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind dei click dei bottoni
	if (Btn_LoginEpic)
	{
		Btn_LoginEpic->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnLoginEpicClicked);
	}
	if (Btn_CancelLogin)
	{
		Btn_CancelLogin->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnCancelLoginClicked);
	}
	if (Btn_Logout)
	{
		Btn_Logout->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnLogoutClicked);
	}
	// Bind del click su "Crea Lobby"
	if (Btn_CreateLobby)
	{
		Btn_CreateLobby->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnCreateLobbyClicked);
	}
	if (Btn_SearchLobby) Btn_SearchLobby->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnSearchLobbyClicked);
	if (Btn_ConfirmCreate) Btn_ConfirmCreate->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnConfirmCreateClicked);
	if (Btn_RefreshList) Btn_RefreshList->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnRefreshListClicked);
	if (Btn_BackFromCreate) Btn_BackFromCreate->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnBackToHubClicked);
	if (Btn_BackFromFind) Btn_BackFromFind->OnClicked.AddDynamic(this, &UStartPcScreenWidgetDRB::OnBackToHubClicked);

	// Bind dell'evento del subsystem per sapere quando il login finisce
	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		EOS_Subsystem->OnEpicLoginComplete.AddDynamic(this, &UStartPcScreenWidgetDRB::HandleLoginComplete);
		// Bind dell'evento di risposta della Lobby
		EOS_Subsystem->OnEpicLobbyCreateComplete.AddDynamic(this, &UStartPcScreenWidgetDRB::HandleLobbyCreated);

		// NUOVO: CONTROLLO STATO LOGIN ATTUALE
		// Se la mappa è appena stata ricaricata, ma noi eravamo già loggati in Epic...
		if (EOS_Subsystem->IsEpicAccountLoggedIn(GetOwningPlayer()))
		{
			// ...salta tutto e vai dritto alla Dashboard (Indice 2)
			if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(2);

			// Ricompila il testo del Nickname
			FString Nickname = EOS_Subsystem->GetUserDisplayName(GetOwningPlayer());
			if (Text_Nickname) Text_Nickname->SetText(FText::FromString(Nickname));

			// Interrompi l'esecuzione per non far partire l'autologin o tornare a Indice 0
			return;
		}
	}

	// Controllo del SaveGame per l'Auto-Login
	if (USettingsSaveDRB* SaveSlot = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0)))
	{
		if (SaveSlot->bAutoLoginEnabled)
		{
			// Mettiamo il monitor in stato di "Attesa" visiva
			if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(1);

			// Avviamo il login in background. Passando "true", il Subsystem userà PersistentAuth!
			if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
			{
				EOS_Subsystem->LoginWithEpic(GetOwningPlayer(), true);
			}
			// Interruzione dell'esecuzione per non mostrare il pannello di pre-login
			return;
		}
	}

	// Se il file di salvataggio non esiste o l'autologin è disabilitato,
	// Inizializza lo stato: Mostra il pannello di Pre-Login
	if (ScreenSwitcher)
	{
		ScreenSwitcher->SetActiveWidgetIndex(0);
	}
}


void UStartPcScreenWidgetDRB::OnLoginEpicClicked()
{
	// 1. Legge lo stato della checkbox per il savegame (da implementare in BP o C++)
	bool bRememberMe = CheckBox_RememberMe->IsChecked();

	USettingsSaveDRB* SettingsSave = nullptr;
	// Proviamo a caricare il file esistente
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSaveDRB"), 0))
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0));
	}
	// Se non esiste o il caricamento è fallito, ne creiamo uno nuovo
	if (!SettingsSave)
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass()));
	}
	// Aggiorniamo la variabile dell'autologin con la scelta del giocatore e salviamo su disco
	if (SettingsSave && SettingsSave->bAutoLoginEnabled != bRememberMe)
	{
		SettingsSave->bAutoLoginEnabled = bRememberMe; // (o false nei casi di logout/errore)
		UGameplayStatics::SaveGameToSlot(SettingsSave, TEXT("SettingsSaveDRB"), 0);
	}

	/* SISTEMA CHE CREA OGNI VOLTA IL SAVE FILE
	// Creiamo o sovrascriviamo il SaveGame con la scelta del giocatore
	if (USettingsSaveDRB* SaveSlot = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass())))
	{
		SaveSlot->bAutoLoginEnabled = bRememberMe;
		UGameplayStatics::SaveGameToSlot(SaveSlot, TEXT("SettingsSaveDRB"), 0);
	}
	*/

	// 2. Cambia lo schermo sul pannello dell'attesa login (Indice 1)
	if (ScreenSwitcher)
	{
		ScreenSwitcher->SetActiveWidgetIndex(1);
	}

	// 3. Avvia la richiesta di Login tramite il Subsystem
	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		// Usiamo il GetOwningPlayer() che ci restituisce il controller
		//EOS_Subsystem->LoginWithEpic(GetOwningPlayer(), bRememberMe); //prima di provare a sistemare il tasto remember me

		// Passando "false", forziamo AccountPortal (apertura browser) per questo primo accesso manuale
		EOS_Subsystem->LoginWithEpic(GetOwningPlayer(), false);
	}
}

void UStartPcScreenWidgetDRB::HandleLoginComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// Se il gioco è già un Listen Server (es. avviato dal Main Menu), non serve ricaricare nulla!
		if (GetWorld()->GetNetMode() == NM_ListenServer)
		{
			// Mostra la dashboard Lobby (Indice 2)
			if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(2);

			// Aggiorna il testo del nickname
			if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
			{
				FString Nickname = EOS_Subsystem->GetUserDisplayName(GetOwningPlayer());
				Text_Nickname->SetText(FText::FromString(Nickname));
			}
		}
		else
		{
			// Eravamo in una mappa offline. Ricarichiamo il livello per accendere il NetDriver!
			UE_LOG(LogTemp, Warning, TEXT("Login in-game effettuato. Ricaricamento mappa in modalità Listen Server..."));
			FName MapName = TEXT("Lvl_FirstPerson"); // Inserisci il nome esatto della tua mappa
			FString Options = TEXT("listen");
			UGameplayStatics::OpenLevel(this, MapName, true, Options);
		}
	}
	else
	{

		USettingsSaveDRB* SettingsSave = nullptr;
		// Proviamo a caricare il file esistente
		if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSaveDRB"), 0))
		{
			SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0));
		}
		// Se non esiste o il caricamento è fallito, ne creiamo uno nuovo
		if (!SettingsSave)
		{
			SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass()));
		}
		// Se il login in background fallisce (es. token scaduto), resettiamo il SettingsSaveDRB e salviamo su disco
		if (SettingsSave)
		{
			SettingsSave->bAutoLoginEnabled = false;
			UGameplayStatics::SaveGameToSlot(SettingsSave, TEXT("SettingsSaveDRB"), 0);
		}

		/* SISTEMA CHE CREA OGNI VOLTA IL SAVE FILE
		// Se il login in background fallisce (es. token scaduto), resettiamo il SaveGame
		if (USettingsSaveDRB* SaveSlot = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass())))
		{
			SaveSlot->bAutoLoginEnabled = false;
			UGameplayStatics::SaveGameToSlot(SaveSlot, TEXT("SettingsSaveDRB"), 0);
		}
		*/

		// Se fallisce, torna alla schermata iniziale
		if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(0);
	}
	// Riporta l'attenzione di Windows e di Unreal su questo widget
	SetKeyboardFocus();
}

void UStartPcScreenWidgetDRB::OnCancelLoginClicked()
{
	// Riporta lo schermo al pannello di pre-login
	if (ScreenSwitcher)
	{
		ScreenSwitcher->SetActiveWidgetIndex(0);
	}

	// NOTA: L'SDK di Epic in background potrebbe rimanere in attesa fino al timeout,
	// ma almeno il giocatore ora è libero di muoversi, usare gli altri menu 
	// o uscire dal PC premendo 'E' //ESC.

	//Reset dell'autologin nel SettingsSaveDRB, forzando un accesso manuale per la prossima volta
	USettingsSaveDRB* SettingsSave = nullptr;
	// Proviamo a caricare il file esistente
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSaveDRB"), 0))
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0));
	}
	// Se non esiste o il caricamento è fallito, ne creiamo uno nuovo
	if (!SettingsSave)
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass()));
	}
	// Aggiorniamo la variabile dell'autologin e salviamo su disco
	if (SettingsSave)
	{
		// Cancella l'autologin futuro dal disco
		SettingsSave->bAutoLoginEnabled = false;
		UGameplayStatics::SaveGameToSlot(SettingsSave, TEXT("SettingsSaveDRB"), 0);
	}
}

void UStartPcScreenWidgetDRB::OnLogoutClicked()
{

	USettingsSaveDRB* SettingsSave = nullptr;
	// Proviamo a caricare il file esistente
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSaveDRB"), 0))
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSaveDRB"), 0));
	}
	// Se non esiste o il caricamento è fallito, ne creiamo uno nuovo
	if (!SettingsSave)
	{
		SettingsSave = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass()));
	}
	// Aggiorniamo la variabile dell'autologin e salviamo su disco
	if (SettingsSave)
	{
		// Cancella l'autologin futuro dal disco
		SettingsSave->bAutoLoginEnabled = false;
		UGameplayStatics::SaveGameToSlot(SettingsSave, TEXT("SettingsSaveDRB"), 0);
	}

	/* SISTEMA CHE CREA OGNI VOLTA IL SAVE FILE
	// Cancella l'autologin futuro dal disco
	if (USettingsSaveDRB* SaveSlot = Cast<USettingsSaveDRB>(UGameplayStatics::CreateSaveGameObject(USettingsSaveDRB::StaticClass())))
	{
		SaveSlot->bAutoLoginEnabled = false;
		UGameplayStatics::SaveGameToSlot(SaveSlot, TEXT("SettingsSaveDRB"), 0);
	}
	*/

	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		EOS_Subsystem->LogoutEpicAccount(GetOwningPlayer());
	}

	// Torna allo stato Pre-Login
	if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(0);
}

/* PER USCIRE CON TASTO ESC
FReply UStartPcScreenWidgetDRB::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Controlla se il tasto premuto è ESC
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// Otteniamo il Player Controller
		if (APlayerController* PC = GetOwningPlayer())
		{
			// Otteniamo il Pawn (il nostro personaggio) per sapere dove riportare la telecamera
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				// 1. Riporta la telecamera al personaggio con la stessa transizione fluida di 0.7s
				PC->SetViewTargetWithBlend(PlayerPawn, 0.7f, EViewTargetBlendFunction::VTBlend_Cubic);

				// 2. Nascondi il cursore del mouse
				PC->SetShowMouseCursor(false);

				// 3. Ripristina l'input per muovere il personaggio (Ignorando la UI)
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
			}
		}

		// Restituiamo Handled per dire a Unreal: "Ho gestito io questo input, ferma la propagazione"
		return FReply::Handled();
	}

	// Se è stato premuto un altro tasto, passa il comando alla classe base (comportamento standard)
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
*/

FReply UStartPcScreenWidgetDRB::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Controlla se il tasto premuto è 'E'
	if (InKeyEvent.GetKey() == EKeys::E)
	{
		// Otteniamo il Player Controller
		if (APlayerController* PC = GetOwningPlayer())
		{
			// Otteniamo il Pawn (il nostro personaggio) per sapere dove riportare la telecamera
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				// 1. Riporta la telecamera al personaggio con la stessa transizione fluida di 0.7s
				PC->SetViewTargetWithBlend(PlayerPawn, 0.7f, EViewTargetBlendFunction::VTBlend_Cubic);

				// 2. Nascondi il cursore del mouse
				PC->SetShowMouseCursor(false);

				// 3. Ripristina l'input per muovere il personaggio (Ignorando la UI)
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
			}
		}

		// Restituiamo Handled per dire a Unreal: "Ho gestito io questo input, ferma la propagazione"
		return FReply::Handled();
	}

	// Se è stato premuto un altro tasto, passa il comando alla classe base (comportamento standard)
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UStartPcScreenWidgetDRB::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// Quando il mouse sfiora lo schermo 3D, forziamo il gioco a ridare il focus della tastiera a questo widget.
	// Questo risolve il problema della perdita di input dopo un "Alt-Tab" o dopo aver usato il browser esterno.
	SetKeyboardFocus();
}

FReply UStartPcScreenWidgetDRB::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 1. Riprendiamo forzatamente il focus della tastiera per il tasto 'E'
	SetKeyboardFocus();

	// 2. Restituiamo "Handled". Questo dice a Unreal: 
	// "Ho gestito io questo click, NON passarlo al Viewport del gioco!"
	return FReply::Handled();
}

// LOBBY
/* Prima versione
void UStartPcScreenWidgetDRB::OnCreateLobbyClicked()
{
	// 1. Facciamo sparire la UI per evitare doppi click o click accidentali durante il caricamento
	// Puoi rimandare lo schermo a una schermata di caricamento, se ce l'hai, oppure disabilitare temporaneamente il bottone
	Btn_CreateLobby->SetIsEnabled(false);

	// 2. Chiamiamo il Subsystem per creare la lobby (Massimo 4 giocatori, Pubblica)
	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		// Passiamo 'false' per farla pubblica (PublicAdvertised). Se vuoi farla privata, passa 'true'
		EOS_Subsystem->CreateEpicLobby(GetOwningPlayer(), "Prova", 4, false);
	}
}


void UStartPcScreenWidgetDRB::HandleLobbyCreated(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// LA MAGIA DEL P2P: TRASFORMIAMO IL GIOCO IN UN SERVER!

		// Nome esatto dell'asset della mappa (es. "Level_Hub")
		FName MapName = TEXT("Lvl_FirstPerson");

		// Questa opzione dice a Unreal di ricaricare il livello, ma questa volta accendendo i protocolli di rete!
		FString Options = TEXT("listen");

		//UGameplayStatics::OpenLevel(this, MapName, true, Options);
	}
	else
	{
		// Se fallisce, riattiviamo il bottone e magari in futuro stamperemo un messaggio a schermo
		Btn_CreateLobby->SetIsEnabled(true);
	}
}
*/

void UStartPcScreenWidgetDRB::OnCreateLobbyClicked()
{
	// Non creiamo più la lobby subito, ma spostiamo lo Switcher sul pannello di Creazione (Indice 3)
	if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(3);
}

void UStartPcScreenWidgetDRB::OnConfirmCreateClicked()
{
	Btn_ConfirmCreate->SetIsEnabled(false);

	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		// Leggiamo il testo dal TextBox. Se è vuoto, mettiamo un nome di default
		FString LobbyName = TextBox_LobbyName->GetText().ToString();//->GetText();//.ToString();
		if (LobbyName.IsEmpty())
		{
			LobbyName = TEXT("Lobby di ") + EOS_Subsystem->GetUserDisplayName(GetOwningPlayer());
		}

		// Creiamo la lobby (4 Giocatori, Pubblica=0)
		EOS_Subsystem->CreateEpicLobby(GetOwningPlayer(), LobbyName, 4, 0);
	}
}

void UStartPcScreenWidgetDRB::HandleLobbyCreated(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// NIENTE CARICAMENTI! La stanza viene creata silenziosamente nel cloud.
		UE_LOG(LogTemp, Log, TEXT("Lobby Cloud creata! Il giocatore è già un Listen Server."));

		// Riporta lo Switcher al pannello Hub (Indice 2)
		if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(2);

		// Opzionale: potresti riabilitare il tasto nel caso il giocatore la distrugga e voglia ricrearla
		if (Btn_ConfirmCreate) Btn_ConfirmCreate->SetIsEnabled(true);
	}
	else
	{
		// Se fallisce, riattiviamo il bottone
		if (Btn_ConfirmCreate) Btn_ConfirmCreate->SetIsEnabled(true);
		UE_LOG(LogTemp, Error, TEXT("ERRORE: La creazione su EOS è fallita! Controlla l'Output Log."));
	}
}

void UStartPcScreenWidgetDRB::OnSearchLobbyClicked()
{
	// Vai alla schermata di ricerca (Indice 4)
	if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(4);

	// Avvia subito una prima scansione automatica simulando il click su Refresh
	OnRefreshListClicked();
}

void UStartPcScreenWidgetDRB::OnRefreshListClicked()
{
	if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(false);

	if (UEosGameInstanceSubsystemDRB* EOS_Subsystem = GetGameInstance()->GetSubsystem<UEosGameInstanceSubsystemDRB>())
	{
		EOS_Subsystem->FindEpicLobbies(GetOwningPlayer());
	}
}

void UStartPcScreenWidgetDRB::OnBackToHubClicked()
{
	// Tasto indietro: riporta lo Switcher alla Dashboard (Indice 2)
	if (ScreenSwitcher) ScreenSwitcher->SetActiveWidgetIndex(2);
}