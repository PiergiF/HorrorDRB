#include "EosGameInstanceSubsystemDRB.h"

#include "Online/CoreOnline.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineError.h"
#include "Online/OnlineServices.h"
#include "Online/Auth.h"
#include "Online/TitleFile.h"

DEFINE_LOG_CATEGORY(LogEosGameInstanceSubsystemDRB);

/// <summary>
/// Whether to create this subsystem. For simplicity, the subsystem is only
///		created on clients and standalone games, not servers. This function
///		is often used to limit creation of subsystems to a server or client.
///		Be sure to null-check subsystem before usage!
/// </summary>
/// <param name="Outer"></param>
/// <returns>Boolean whether or not to create this subsystem</returns>
bool UEosGameInstanceSubsystemDRB::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_SERVER
	return false;
#else
	return Super::ShouldCreateSubsystem(Outer);
#endif
}

/// <summary>
/// Initialize called after the Game Instance is initialized
/// </summary>
/// <param name="Collection">Collection of subsystems that are initialized by the game instance</param>
void UEosGameInstanceSubsystemDRB::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogTemp, Log, TEXT("OnlineSampleOnlineSubsystem initialized."));
	Super::Initialize(Collection);

	// Initialize online services
	InitializeOnlineServices();
}

/// <summary>
/// Deinitialize called before the Game Instance is deinitialized/shutdown
/// </summary>
void UEosGameInstanceSubsystemDRB::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("OnlineSampleOnlineSubsystem deinitialized."));

	/// --- DRB - LOBBY GPT --- ///
	if (OnlineServicesInfoInternal)
	{
		OnlineServicesInfoInternal->ExternalUIEventHandle.Unbind();
		OnlineServicesInfoInternal->LobbyInviteHandle.Unbind();

		OnlineServicesInfoInternal->Reset();
		
		CachedActiveLobby.Reset();
	}
	/// ^^^ DRB - LOBBY GPT ^^^ ///

	// Unbind event handles and reset struct info
	//OnlineServicesInfoInternal->Reset(); ORIGINALE

	///--- DRB - ACCOUNT ---///
	/// Garbage collector
	// per troncare i riferimenti alla chiusura del subsystem
	OnlineUserInfos.Empty();
	///^^^ DRB - ACCOUNT ^^^///

	// Deinitialize parent class
	Super::Deinitialize();
}

/// <summary>
/// Handle the asynchronous EnumerateFiles function. Log if there is a failure.
///		Success calls the asynchronous ReadFile function handled by HandleReadFile.
/// </summary>
/// <param name="EnumerateFilesResult">Result of Enumerate Files attempt</param>
/// <param name="OnlineUser">User that queried for title file</param>
/// <param name="Filename">Name of file user queried</param>
void UEosGameInstanceSubsystemDRB::HandleEnumerateFiles(const UE::Online::TOnlineResult<UE::Online::FTitleFileEnumerateFiles>& EnumerateFilesResult, TObjectPtr<UOnlineUserInfo> OnlineUser, FString Filename)
{
	using namespace UE::Online;

	if (EnumerateFilesResult.IsOk())
	{
		FTitleFileGetEnumeratedFiles::Params GetParams;
		GetParams.LocalAccountId = OnlineUser->AccountId;

		TOnlineResult<FTitleFileGetEnumeratedFiles> GetResult = OnlineServicesInfoInternal->TitleFileInterface->GetEnumeratedFiles(MoveTemp(GetParams));
		if (GetResult.IsOk())
		{
			FTitleFileGetEnumeratedFiles::Result& CachedFiles = GetResult.GetOkValue();
			int32 FileIndex = CachedFiles.Filenames.Find(Filename);
			if (FileIndex == INDEX_NONE)
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Title File \"%s\" not found!"), *Filename);
			}
			else
			{
				FTitleFileReadFile::Params ReadParams;
				ReadParams.LocalAccountId = OnlineUser->AccountId;
				ReadParams.Filename = Filename;

				OnlineServicesInfoInternal->TitleFileInterface->ReadFile(MoveTemp(ReadParams)).OnComplete(this, &ThisClass::HandleReadFile, Filename);
			}
		}
		else
		{
			UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Get Title File Error: %s"), *GetResult.GetErrorValue().GetLogString());
		}
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Enum Title File Error: %s"), *EnumerateFilesResult.GetErrorValue().GetLogString());
	}
}

/// <summary>
/// Handle the asynchronous ReadFile function. Log if there is a failure.
///		Success populates the user's TitleFileContent.
/// </summary>
/// <param name="ReadFileResult">Result of ReadFile attempt</param>
/// <param name="Filename">Name of file to read</param>
void UEosGameInstanceSubsystemDRB::HandleReadFile(const UE::Online::TOnlineResult<UE::Online::FTitleFileReadFile>& ReadFileResult, FString Filename)
{
	using namespace UE::Online;

	if (ReadFileResult.IsOk())
	{
		const FTitleFileReadFile::Result& ReadFileResultValue = ReadFileResult.GetOkValue();
		OnlineServicesInfoInternal->TitleFileContent = *ReadFileResultValue.FileContents;
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Read Title File Error: %s"), *ReadFileResult.GetErrorValue().GetLogString());
	}
}

/// <summary>
/// Initialize the Online Services:
///		Obtain pointer to services
///		Obtain pointers to interfaces
///		Add event handles
///		Check pointer validity
/// </summary>
void UEosGameInstanceSubsystemDRB::InitializeOnlineServices()
{
	OnlineServicesInfoInternal = new FOnlineServicesInfo();

	// Initialize Services ptr
	OnlineServicesInfoInternal->OnlineServices = UE::Online::GetServices();
	check(OnlineServicesInfoInternal->OnlineServices.IsValid());

	// Verify Services type
	OnlineServicesInfoInternal->OnlineServicesType = OnlineServicesInfoInternal->OnlineServices->GetServicesProvider();
	if (OnlineServicesInfoInternal->OnlineServices.IsValid())
	{
		// Initialize Interface ptrs
		OnlineServicesInfoInternal->AuthInterface = OnlineServicesInfoInternal->OnlineServices->GetAuthInterface();
		check(OnlineServicesInfoInternal->AuthInterface.IsValid());

		OnlineServicesInfoInternal->TitleFileInterface = OnlineServicesInfoInternal->OnlineServices->GetTitleFileInterface();
		check(OnlineServicesInfoInternal->TitleFileInterface.IsValid());

		///--- DRB - ACCOUNT ---///
		OnlineServicesInfoInternal->UserInfoInterface = OnlineServicesInfoInternal->OnlineServices->GetUserInfoInterface();
		OnlineServicesInfoInternal->SocialInterface = OnlineServicesInfoInternal->OnlineServices->GetSocialInterface();
		check(OnlineServicesInfoInternal->SocialInterface.IsValid());
		OnlineServicesInfoInternal->PresenceInterface = OnlineServicesInfoInternal->OnlineServices->GetPresenceInterface();
		// INIZIALIZZA EXTERNAL UI E AGGANCIA L'EVENTO
		OnlineServicesInfoInternal->ExternalUIInterface = OnlineServicesInfoInternal->OnlineServices->GetExternalUIInterface();
		if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
		{
			OnlineServicesInfoInternal->ExternalUIInterface = OnlineServicesInfoInternal->OnlineServices->GetExternalUIInterface();
		}
		///^^^ DRB - ACCOUNT ^^^///
		///
		///--- DRB - LOBBY ---///
		// Inizializza l'interfaccia delle Lobbies
		OnlineServicesInfoInternal->LobbiesInterface = OnlineServicesInfoInternal->OnlineServices->GetLobbiesInterface();
		if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
		{
			UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore: Impossibile inizializzare LobbiesInterface."));
		}
		///^^^ DRB - LOBBY ^^^///
		///
		/// --- DRB - SESSION --- ///
		OnlineServicesInfoInternal->SessionsInterface = OnlineServicesInfoInternal->OnlineServices->GetSessionsInterface();
		if (!OnlineServicesInfoInternal->SessionsInterface.IsValid())
		{
			UE_LOG(
				LogEosGameInstanceSubsystemDRB,
				Warning,
				TEXT("SessionsInterface non disponibile.")
			);
		}
		else
		{
			UE_LOG(
				LogEosGameInstanceSubsystemDRB,
				Log,
				TEXT("SessionsInterface inizializzata correttamente.")
			);
		}
		/// ^^^ DRB - SESSION ^^^ ///
	}
	else {
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Error: Failed to initialize services."));
	}
}

/// <summary>
/// EnumerateFiles, GetEnumeratedFiles, and ReadFile. This implementation uses a lambda function
///		to handle the OnComplete callback.
/// </summary>
/// <param name="Filename">File to read</param>
/// <param name="PlatformUserId">User to retrieve file for</param>
void UEosGameInstanceSubsystemDRB::RetrieveTitleFile(FString Filename, FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	FTitleFileEnumerateFiles::Params EnumParams;
	FAccountId LocalAccountId;
	TObjectPtr<UOnlineUserInfo> OnlineUser;
	if (OnlineUserInfos.Contains(PlatformUserId))
	{
		OnlineUser = *OnlineUserInfos.Find(PlatformUserId);
		LocalAccountId = OnlineUser->AccountId;
		EnumParams.LocalAccountId = LocalAccountId;
		if (OnlineServicesInfoInternal->TitleFileInterface.IsValid())
		{
			(OnlineServicesInfoInternal->TitleFileInterface)->EnumerateFiles(MoveTemp(EnumParams)).OnComplete(this, &ThisClass::HandleEnumerateFiles, OnlineUser, Filename);
		}
		else
		{
			UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Title File Interface pointer invalid."));
		}
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Could not find user with Platform User Id: %d"), PlatformUserId.GetInternalId());
	}
}

/// <summary>
/// Register online user with local registry OnlineUserInfos
/// </summary>
/// <param name="PlatformUserId">Platform user id of user to register</param>
void UEosGameInstanceSubsystemDRB::RegisterLocalOnlineUser(FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	FAuthGetLocalOnlineUserByPlatformUserId::Params GetUserParams;
	GetUserParams.PlatformUserId = PlatformUserId;
	if (OnlineServicesInfoInternal->AuthInterface.IsValid())
	{
		TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> AuthGetResult = OnlineServicesInfoInternal->AuthInterface->GetLocalOnlineUserByPlatformUserId(MoveTemp(GetUserParams));

		if (AuthGetResult.IsOk())
		{
			FAuthGetLocalOnlineUserByPlatformUserId::Result& LocalOnlineUser = AuthGetResult.GetOkValue();
			TSharedRef<FAccountInfo> UserAccountInfo = LocalOnlineUser.AccountInfo;
			FAccountInfo UserAccountInfoContent = *UserAccountInfo;
			if (!OnlineUserInfos.Contains(UserAccountInfoContent.PlatformUserId))
			{
				UOnlineUserInfo* NewUser = CreateAndRegisterUserInfo(UserAccountInfoContent.AccountId.GetHandle(), PlatformUserId, UserAccountInfoContent.AccountId, UserAccountInfoContent.AccountId.GetOnlineServicesType());

				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Local User Registered: %s"), *(NewUser->DebugInfoToString()));
			}
			else
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Local User with platform user id %d already registered."), PlatformUserId.GetInternalId());
			}
		}
		else
		{
			UE::Online::FOnlineError ErrorResult = AuthGetResult.GetErrorValue();
			UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Get Local Online User Error: %s"), *ErrorResult.GetLogString());
		}
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Auth Interface pointer invalid."));
	}
}

/// <summary>
/// Obtain title file and read its contents.
/// </summary>
/// <param name="Filename">File to read</param>
/// <param name="PlatformUserId">User to read file for</param>
/// <returns>FileString with contents of Filename</returns>
FString UEosGameInstanceSubsystemDRB::ReadTitleFile(FString Filename, FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	RetrieveTitleFile(Filename, PlatformUserId);
	FTitleFileContents FileContents = OnlineServicesInfoInternal->TitleFileContent;
	FString FileString = FString(FileContents.Num(), UTF8_TO_TCHAR(FileContents.GetData()));
	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Reading Title File: %s"), *Filename);
	return FileString;
}

/// <summary>
/// Create a UOnlineUserInfo object from the given information.
/// </summary>
/// <param name="LocalUserIndex"></param>
/// <param name="PlatformUserId"></param>
/// <param name="AccountId"></param>
/// <param name="Services">Online services user is registered with</param>
/// <returns>Object pointer to the NewUser</returns>
TObjectPtr<UOnlineUserInfo> UEosGameInstanceSubsystemDRB::CreateOnlineUserInfo(int32 LocalUserIndex, FPlatformUserId PlatformUserId, UE::Online::FAccountId AccountId, UE::Online::EOnlineServices Services)
{
	TObjectPtr<UOnlineUserInfo> NewUser = NewObject<UOnlineUserInfo>(this);
	NewUser->LocalUserIndex = LocalUserIndex;
	NewUser->PlatformUserId = PlatformUserId;
	NewUser->AccountId = AccountId;
	NewUser->Services = Services;
	return NewUser;
}

/// <summary>
/// Create a UOnlineUserInfo object by calling CreateOnlineUserInfo then
///		register the user with the local registry OnlineUserInfos
/// </summary>
/// <param name="LocalUserIndex"></param>
/// <param name="PlatformUserId"></param>
/// <param name="AccountId"></param>
/// <param name="Services">Online services user is registered with</param>
/// <returns>Object pointer to the NewUser</returns>
TObjectPtr<UOnlineUserInfo> UEosGameInstanceSubsystemDRB::CreateAndRegisterUserInfo(int32 LocalUserIndex, FPlatformUserId PlatformUserId, UE::Online::FAccountId AccountId, UE::Online::EOnlineServices Services)
{
	TObjectPtr<UOnlineUserInfo> NewUser = CreateOnlineUserInfo(LocalUserIndex, PlatformUserId, AccountId, Services);
	OnlineUserInfos.Add(PlatformUserId, NewUser);
	return NewUser;
}

/// <summary>
/// Get UOnlineUserInfo for provided platform user id.
/// </summary>
/// <param name="PlatformUserId">id of user to retrieve</param>
/// <returns>Object pointer to OnlineUser</returns>
TObjectPtr<UOnlineUserInfo> UEosGameInstanceSubsystemDRB::GetOnlineUserInfo(FPlatformUserId PlatformUserId)
{
	TObjectPtr<UOnlineUserInfo> OnlineUser;
	if (OnlineUserInfos.Contains(PlatformUserId))
	{
		OnlineUser = *OnlineUserInfos.Find(PlatformUserId);
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Could not find user with Platform User Id: %d"), PlatformUserId.GetInternalId());
		OnlineUser = nullptr;
	}
	return OnlineUser;
}

///--- DRB - ACCOUNT ---///

void UEosGameInstanceSubsystemDRB::HandleLoginComplete(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& LoginResult, FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	if (LoginResult.IsOk())
	{
		const FAuthLogin::Result& ResultValue = LoginResult.GetOkValue();

		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Login EOS (OSSv2) completato con successo! Account ID: %s"), *ToLogString(ResultValue.AccountInfo->AccountId));
		
		RegisterLocalOnlineUser(PlatformUserId);
		
		OnEpicLoginComplete.Broadcast(true);
		

		if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
		{
			// Evitiamo di registrare più volte lo stesso delegate.
			//if (OnlineServicesInfoInternal->ExternalUIEventHandle.IsValid())
			//{
			OnlineServicesInfoInternal->ExternalUIEventHandle.Unbind();
			//}

			OnlineServicesInfoInternal->ExternalUIEventHandle = OnlineServicesInfoInternal->ExternalUIInterface->OnExternalUIStatusChanged().Add([this](const FExternalUIStatusChanged& EventParams)
				{
						HandleExternalUIStatusChanged(EventParams);
				});
		}
		

		if (OnlineServicesInfoInternal->LobbiesInterface.IsValid())
		{
			// Evitiamo un secondo binding dello stesso evento.
			//if (OnlineServicesInfoInternal->LobbyInviteHandle.IsValid())
			//{
			OnlineServicesInfoInternal->LobbyInviteHandle.Unbind();
			//}

			OnlineServicesInfoInternal->LobbyInviteHandle = OnlineServicesInfoInternal->LobbiesInterface->OnUILobbyJoinRequested().Add(
				[this](const FUILobbyJoinRequested& InviteData)
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione Lobby ricevuta dall'Overlay."));

					if (!InviteData.Result.IsOk())
					{
						const FOnlineError ErrorResult = InviteData.Result.GetErrorValue();
						UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore nella richiesta di unione dalla UI: %s"), *ErrorResult.GetLogString());
						return;
					}

					const TSharedRef<const FLobby> TargetLobby = InviteData.Result.GetOkValue();

					const FLobbyId LobbyIdToJoin = TargetLobby->LobbyId;

					// Il callback dell'Overlay non fornisce un APlayerController.
					// Nel nostro gioco abbiamo un solo local player, quindi
					// recuperiamo il suo controller dal World.
					UWorld* World = GetWorld();

					if (!World)
					{
						UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Overlay Invite: World non disponibile."));
						return;
					}

					APlayerController* PlayerController = World->GetFirstPlayerController();

					if (!PlayerController)
					{
						UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Overlay Invite: PlayerController non disponibile."));
						return;
					}

					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Invito accettato. LobbyId: %s"), *ToLogString(LobbyIdToJoin));

					JoinEpicLobby(LobbyIdToJoin, PlayerController);
				}
			);
		}

		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Autenticazione OSSv2 pronta. OnlineUser registrato e servizi disponibili."));
	}
	else
	{
		const FOnlineError ErrorResult = LoginResult.GetErrorValue();

		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore durante il Login EOS: %s"), *ErrorResult.GetLogString());

		OnEpicLoginComplete.Broadcast(false);
	}
}



void UEosGameInstanceSubsystemDRB::LoginWithEpic(APlayerController* PlayerController, bool bAutoLogin)
{
	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	using namespace UE::Online;
	if (!OnlineServicesInfoInternal->AuthInterface.IsValid()) return;

	FAuthLogin::Params LoginParams;
	LoginParams.PlatformUserId = PlatformUserId;


	//---- UNIONE MACRO E VERIFICA IN EDITOR E STADALONE COSI' DA TESTARE SIA AUTH DEVELOPER CHE ACCOUNTPORTAL  --- DA RIMETTERE ALLA FINE ----
#if UE_EDITOR // Questo codice viene compilato SOLO nell'Editor

	// controlliamo a runtime il tipo di simulazione
	if (PlayerController->GetWorld()->IsPlayInEditor())
	{
		// Play In Editor (PIE): Login istantaneo col DevAuthTool
		LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::Developer;
		LoginParams.CredentialsId = TEXT("localhost:8081"); // CredentialsId è solitamente una semplice stringa per indicare la porta locale
		LoginParams.CredentialsToken.Set<FString>(FString(TEXT("Piergi_F"))); // CredentialsToken è un TVariant. Usiamo .Set<FString>() per l'assegnazione sicura
	}
	else
	{
		// Play in Standalone: Test realistico del portale web
		LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::AccountPortal;
	}

#else

	// Se bAutoLogin è true, tenta di usare il token salvato in locale. 
	// Altrimenti apre il portale Epic nel browser/overlay.
	LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::AccountPortal;

#endif

	OnlineServicesInfoInternal->AuthInterface->Login(MoveTemp(LoginParams))
		.OnComplete(this, &ThisClass::HandleLoginComplete, PlatformUserId);

}


void UEosGameInstanceSubsystemDRB::LogoutEpicAccount(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();
	
	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("PlatformUserId.internalId inizio Logout: %d"), PlatformUserId.GetInternalId());

	using namespace UE::Online;
	if (!OnlineServicesInfoInternal->AuthInterface.IsValid()) return;

	// Distrugge la lobby prima di sloggare, per evitare istanze fantasma
	DestroyEpicLobby(PlayerController); /// DRB - LOBBY

	// Recupera l'AccountId dal nostro utente registrato in locale
	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FAuthLogout::Params LogoutParams;
	LogoutParams.LocalAccountId = UserInfo->AccountId;
	
	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("AccountId Logout: %s"), *ToLogString(LogoutParams.LocalAccountId));
	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("LogoutParams Logout: %s"), *ToLogString(LogoutParams));

	// Esegue il logout asincrono
	OnlineServicesInfoInternal->AuthInterface->Logout(MoveTemp(LogoutParams))
		.OnComplete([this, PlatformUserId](const TOnlineResult<FAuthLogout>& LogoutResult)
			{
				if (LogoutResult.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Logout completato con successo."));
					// Rimuovi l'utente dalla nostra mappa locale
					OnlineUserInfos.Remove(PlatformUserId);

					// sgancio overlay
					if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
					{
						// Nel framework V2, chiamiamo semplicemente Unbind() direttamente sull'Handle!
						OnlineServicesInfoInternal->ExternalUIEventHandle.Unbind();
					}

					// Delegato: Il logout ha avuto successo (True)
					OnEpicLogoutComplete.Broadcast(true);
				}
				else
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore Logout: %s"), *LogoutResult.GetErrorValue().GetLogString());

					// Delegato: Il logout e' fallito (False)
					OnEpicLogoutComplete.Broadcast(false);
				}
			});
}

FString UEosGameInstanceSubsystemDRB::GetUserDisplayName(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->GetLocalPlayer()) return TEXT("Sconosciuto");

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	using namespace UE::Online;

	// Otteniamo le informazioni dell'utente autenticato
	FAuthGetLocalOnlineUserByPlatformUserId::Params GetUserParams;
	GetUserParams.PlatformUserId = PlatformUserId;

	if (OnlineServicesInfoInternal->AuthInterface.IsValid())
	{
		TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> AuthGetResult = OnlineServicesInfoInternal->AuthInterface->GetLocalOnlineUserByPlatformUserId(MoveTemp(GetUserParams));

		if (AuthGetResult.IsOk())
		{
			// Recupera l'AccountInfo, che contiene gli attributi del profilo
			TSharedRef<FAccountInfo> UserAccountInfo = AuthGetResult.GetOkValue().AccountInfo;

			// Se il backend supporta il DisplayName base, lo troviamo qui come FSchemaVariant
			if (const UE::Online::FSchemaVariant* DisplayNameVariant = UserAccountInfo->Attributes.Find(TEXT("DisplayName")))
			{
				// Estraiamo e restituiamo la stringa contenuta all'interno della variante
				return DisplayNameVariant->GetString();
			}
		}
	}
	return TEXT("Nickname non trovato");
}

/*
bool UEosGameInstanceSubsystemDRB::IsLoggedWithEpic(FPlatformUserId& PlatformUserId)
{
	return OnlineServicesInfoInternal->AuthInterface->IsLoggedIn(PlatformUserId);
}
*/

bool UEosGameInstanceSubsystemDRB::IsEpicAccountLoggedIn(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->GetLocalPlayer()) return false;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	// Verifica se l'utente è stato registrato localmente dopo un login con successo
	return OnlineUserInfos.Contains(PlatformUserId);
}

//Garbage collector

void UEosGameInstanceSubsystemDRB::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	// Eseguiamo il cast per ottenere l'istanza del nostro Subsystem
	UEosGameInstanceSubsystemDRB* ThisSubsystem = Cast<UEosGameInstanceSubsystemDRB>(InThis);

	if (ThisSubsystem)
	{
		// Iteriamo sulla mappa e diciamo al Collector di proteggere ogni singolo puntatore
		for (auto& Pair : ThisSubsystem->OnlineUserInfos)
		{
			Collector.AddReferencedObject(Pair.Value);
		}
	}

	// È fondamentale chiamare l'implementazione della classe base per non interrompere la catena
	Super::AddReferencedObjects(InThis, Collector);
}


void UEosGameInstanceSubsystemDRB::HandleExternalUIStatusChanged(const UE::Online::FExternalUIStatusChanged& EventParams)
{
	// La variabile che indica se l'overlay è attualmente visibile a schermo
	bool bIsOverlayActive = EventParams.bIsOpening;

	UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("Epic Overlay cambiato. Stato visibilità: %s"), bIsOverlayActive ? TEXT("APERTO") : TEXT("CHIUSO"));

	// Lanciamo l'avviso a tutti i Blueprint in ascolto
	OnExternalUIChange.Broadcast(bIsOverlayActive);
}


///^^^ DRB - ACCOUNT ^^^///
///
///--- DRB - LOBBY ---///
void UEosGameInstanceSubsystemDRB::CreateEpicLobby(APlayerController* PlayerController, FString CustomLobbyName, int32 MaxPlayers, int32 PrivacyType)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	// 1. Controlliamo che l'interfaccia sia valida e che l'utente sia loggato localmente
	if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid()) return;

	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo)
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Impossibile creare la lobby: Utente non loggato."));
		OnEpicLobbyCreateComplete.Broadcast(false);
		return;
	}

	// Prepariamo i parametri della lobby
	FCreateLobby::Params LobbyParams;
	LobbyParams.LocalAccountId = UserInfo->AccountId;
	LobbyParams.MaxMembers = MaxPlayers;

	LobbyParams.bPresenceEnabled = true; //da rivedere insieme alla joi privacy policy

	// Impostiamo se la lobby è pubblica (ricercabile) o privata (solo su invito)
	//LobbyParams.JoinPolicy = bIsPrivate ? ELobbyJoinPolicy::InvitationOnly : ELobbyJoinPolicy::PublicAdvertised; //solo se provate

	ELobbyJoinPolicy Policy = ELobbyJoinPolicy::PublicAdvertised;
	// Nel framework OSSv2, sia "Solo Amici" che "Solo Invito" usano InvitationOnly.
	// Gli amici potranno comunque entrare grazie a bPresenceEnabled = true!
	if (PrivacyType == 1 || PrivacyType == 2) //???POTREI METTERE IL BOOLEANO CHE è IL SOLO SU INVITO, CHE METTE InvitationOnly E METTE A FALSE IL bPresenceEnabled
	{
		Policy = ELobbyJoinPolicy::InvitationOnly;
		if (PrivacyType == 2)
		{
			LobbyParams.bPresenceEnabled = false;
		}
	}

	LobbyParams.JoinPolicy = Policy;

	// AGGANCIO ALLO SCHEMA DELL' .INI
	//LobbyParams.SchemaId = FName(TEXT("GameLobby"));
	LobbyParams.SchemaId = FName(TEXT("HorrorDRB"));

	// Compiliamo gli Attributi della Lobby (Devono coincidere con l'ini!)
	// Creiamo un nome dinamico basato sul nickname di chi la crea
	//FString NomeLobby = GetUserDisplayName(PlayerController) + TEXT("'s Lobby");
	FString ProvaA = TEXT("DAJE ROMA");

	// Compiliamo gli Attributi della Lobby (Devono coincidere con l'ini!)
	LobbyParams.Attributes.Add(FName(TEXT("LobbyName")), CustomLobbyName); // Inseriamo il nome dinamico scelto dal giocatore!
	//LobbyParams.Attributes.Add(FName(TEXT("bIsMatchInProgress")), false); 

	//LobbyParams.Attributes.Add(FName(TEXT("LobbyName")), NomeLobby);
	LobbyParams.Attributes.Add(FName(TEXT("bIsPasswordProtected")), false);
	//LobbyParams.Attributes.Add(FName(TEXT("AoDaje")), TEXT("AOOOOOOOOO"));

	LobbyParams.Attributes.Add(FName(TEXT("GameMode")), CustomLobbyName);
	LobbyParams.Attributes.Add(FName(TEXT("GameSessionId")), ProvaA);

	LobbyParams.Attributes.Add(FName(TEXT("BuildVersion")), TEXT("v0.5.0"));

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Inizio creazione lobby... Nome: %s, Max Giocatori: %d"), *CustomLobbyName, MaxPlayers);

	// Chiamata asincrona a Epic Games
	OnlineServicesInfoInternal->LobbiesInterface->CreateLobby(MoveTemp(LobbyParams))
		.OnComplete([this](const TOnlineResult<FCreateLobby>& Result)
			{
				if (Result.IsOk())
				{
					const FCreateLobby::Result& ResultValue = Result.GetOkValue();
					
					CachedActiveLobby = ResultValue.Lobby;
					
					ActiveLobbyId = ResultValue.Lobby->LobbyId;
					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Lobby creata con successo! Lobby ID: %s; Membri=%d; Leader=%s"), *ToLogString(ActiveLobbyId), CachedActiveLobby->Members.Num(), *ToLogString(CachedActiveLobby->OwnerAccountId));
					
					bHasActiveLobby = true;
					//Update bind
					OnEpicLobbyCreateComplete.Broadcast(true);
					OnEpicLobbyJoinComplete.Broadcast(true);
				}
				else
				{
					UE::Online::FOnlineError ErrorResult = Result.GetErrorValue();
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore durante la creazione della Lobby: %s"), *ErrorResult.GetLogString());

					// Avvisiamo la UI del fallimento
					OnEpicLobbyCreateComplete.Broadcast(false);
				}
			});
}

void UEosGameInstanceSubsystemDRB::DestroyEpicLobby(APlayerController* PlayerController)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;
	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !ActiveLobbyId.IsValid())
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("Nessuna lobby attiva da chiudere."));
		OnEpicLobbyDestroyComplete.Broadcast(false);
		return;
	}

	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FLeaveLobby::Params LeaveParams;
	LeaveParams.LocalAccountId = UserInfo->AccountId;
	LeaveParams.LobbyId = ActiveLobbyId;

	OnlineServicesInfoInternal->LobbiesInterface->LeaveLobby(MoveTemp(LeaveParams))
		.OnComplete([this](const TOnlineResult<FLeaveLobby>& Result)
			{
				if (Result.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Lobby chiusa/abbandonata con successo."));
					// Reset dell'ID memorizzato
					ActiveLobbyId = FLobbyId();
					OnEpicLobbyDestroyComplete.Broadcast(true);
				}
				else
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore chiusura Lobby: %s"), *Result.GetErrorValue().GetLogString());
					OnEpicLobbyDestroyComplete.Broadcast(false);
				}
			});
}

void UEosGameInstanceSubsystemDRB::JoinEpicLobby(UE::Online::FLobbyId LobbyToJoin, APlayerController* PlayerController)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("JoinEpicLobby: PlayerController non valido."));

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->OnlineServices.IsValid() ||
		!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("JoinEpicLobby: Online Services o Lobby Interface non validi."));

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}

	const FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);

	if (!UserInfo)
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("JoinEpicLobby: utente EOS locale non trovato.")
		);

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}

	FJoinLobby::Params JoinParams;
	JoinParams.LocalAccountId = UserInfo->AccountId;
	JoinParams.LobbyId = LobbyToJoin;
	JoinParams.bPresenceEnabled = true;

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Join Lobby in corso: %s"), *ToLogString(LobbyToJoin));

	OnlineServicesInfoInternal->LobbiesInterface->JoinLobby(MoveTemp(JoinParams))
		.OnComplete([this, PlayerController, LocalAccountId = UserInfo->AccountId](const TOnlineResult<FJoinLobby>& Result)
			{
				using namespace UE::Online;

				if (!Result.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("JoinLobby fallita: %s"), *Result.GetErrorValue().GetLogString());

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				const FJoinLobby::Result& ResultValue = Result.GetOkValue();

				if (!ResultValue.Lobby.IsValid())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("JoinLobby riuscita ma FLobby non è valido."));

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}
				
				ActiveLobbyId = ResultValue.Lobby->LobbyId; // Salviamo la Lobby locale.

				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Lobby joinata con successo: %s"), *ToLogString(ActiveLobbyId));

				FGetResolvedConnectString::Params ConnectParams;
				ConnectParams.LocalAccountId = LocalAccountId;
				ConnectParams.LobbyId = ActiveLobbyId;
				ConnectParams.PortType = NAME_GamePort;

				IOnlineServicesPtr OnlineServices = UE::Online::GetServices();

				if (!OnlineServices.IsValid())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Online Services non disponibile."));

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta GetResolvedConnectString per Lobby..."));

				TOnlineResult<FGetResolvedConnectString> ConnectResult = OnlineServicesInfoInternal->OnlineServices->GetResolvedConnectString(MoveTemp(ConnectParams));

				if (!ConnectResult.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("GetResolvedConnectString fallita: %s"), *ConnectResult.GetErrorValue().GetLogString());

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				const FString ConnectString = ConnectResult.GetOkValue().ResolvedConnectString;

				UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("EOS Resolved Connect String = %s"), *ConnectString);

				if (ConnectString.IsEmpty())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Resolved Connect String vuota."));

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}


				UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("NetMode prima del ClientTravel: %d"), (int32)GetWorld()->GetNetMode());
				UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("World URL prima del ClientTravel: %s"),*GetWorld()->URL.ToString());
				UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("ClientTravel URL = %s"), *ConnectString);
				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("ClientTravel verso EOS P2P..."));

				PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);

				// ATTENZIONE:
				// Questo significa che la richiesta di travel è stata
				// avviata, NON che la connessione è già riuscita.
				OnEpicLobbyJoinComplete.Broadcast(true);
			}
		);
}


void UEosGameInstanceSubsystemDRB::FindEpicLobbies(APlayerController* PlayerController)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;
	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid()) return;

	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FFindLobbies::Params FindParams;
	FindParams.LocalAccountId = UserInfo->AccountId;
	FindParams.MaxResults = 20; // Limite di stanze da scaricare

	// FILTRO DI RICERCA: Cerchiamo solo le lobby con il nostro attributo custom
	FFindLobbySearchFilter Filter;
	Filter.AttributeName = FName(TEXT("BuildVersion"));
	Filter.ComparisonOp = ESchemaAttributeComparisonOp::Equals;
	Filter.ComparisonValue = FSchemaVariant(FString(TEXT("v0.5.0"))); // Il valore esatto impostato in CreateLobby

	FindParams.Filters.Add(Filter);

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Avvio ricerca Lobby pubbliche per HorrorDRB..."));

	OnlineServicesInfoInternal->LobbiesInterface->FindLobbies(MoveTemp(FindParams))
		.OnComplete([this](const TOnlineResult<FFindLobbies>& Result)
			{
				if (Result.IsOk())
				{
					// Salviamo la lista nel nostro array locale!
					CurrentSearchResults = Result.GetOkValue().Lobbies;

					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Ricerca completata! Trovate %d lobby."), CurrentSearchResults.Num());
					OnEpicLobbiesSearchComplete.Broadcast(true);
				}
				else
				{
					CurrentSearchResults.Empty();
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore ricerca: %s"), *Result.GetErrorValue().GetLogString());
					OnEpicLobbiesSearchComplete.Broadcast(false);
				}
			});
}

int32 UEosGameInstanceSubsystemDRB::GetFoundLobbiesCount() const
{
	return CurrentSearchResults.Num();
}

FString UEosGameInstanceSubsystemDRB::GetFoundLobbyName(int32 Index) const
{
	if (CurrentSearchResults.IsValidIndex(Index))
	{
		// Estraiamo in modo sicuro l'attributo LobbyName
		if (const UE::Online::FSchemaVariant* NameVariant = CurrentSearchResults[Index]->Attributes.Find(FName(TEXT("LobbyName"))))
		{
			return NameVariant->GetString();
		}
	}
	return TEXT("Lobby Sconosciuta");
}

void UEosGameInstanceSubsystemDRB::JoinEpicLobbyByIndex(APlayerController* PlayerController, int32 Index)
{
	if (CurrentSearchResults.IsValidIndex(Index))
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Avvio unione manuale alla lobby Indice: %d"), Index);

		// Chiamiamo la funzione nativa di join usando l'ID estratto dall'array
		JoinEpicLobby(CurrentSearchResults[Index]->LobbyId, PlayerController);
	}
	else
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Warning, TEXT("Indice Lobby non valido!"));
	}
}

void UEosGameInstanceSubsystemDRB::LeaveEpicLobby(APlayerController* PlayerController)
{
    using namespace UE::Online;

    if (!PlayerController || !PlayerController->GetLocalPlayer())
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Warning,
            TEXT("LeaveEpicLobby: PlayerController non valido.")
        );
        return;
    }

    if (!OnlineServicesInfoInternal ||
        !OnlineServicesInfoInternal->LobbiesInterface.IsValid())
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Error,
            TEXT("LeaveEpicLobby: LobbiesInterface non disponibile.")
        );
        return;
    }

    if (!ActiveLobbyId.IsValid())
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Warning,
            TEXT("LeaveEpicLobby: nessuna Lobby attiva.")
        );
        return;
    }

    const FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

    UOnlineUserInfo* UserInfo = GetOnlineUserInfo(PlatformUserId);

    if (!UserInfo)
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Error,
            TEXT("LeaveEpicLobby: OnlineUserInfo non trovato.")
        );
        return;
    }

    const FAccountId LocalAccountId = UserInfo->AccountId;

    UE_LOG(
        LogEosGameInstanceSubsystemDRB,
        Log,
        TEXT("Uscita dalla Lobby %s richiesta."),
        *ToLogString(ActiveLobbyId)
    );

    FLeaveLobby::Params LeaveParams;

    LeaveParams.LocalAccountId = LocalAccountId;
    LeaveParams.LobbyId = ActiveLobbyId;

    OnlineServicesInfoInternal->LobbiesInterface
        ->LeaveLobby(MoveTemp(LeaveParams))
        .OnComplete(
            [this, LocalAccountId](const TOnlineResult<FLeaveLobby>& Result)
            {
                if (!Result.IsOk())
                {
                    UE_LOG(
                        LogEosGameInstanceSubsystemDRB,
                        Error,
                        TEXT("LeaveLobby fallito: %s"),
                        *ToLogString(Result.GetErrorValue())
                    );

                    return;
                }

                UE_LOG(
                    LogEosGameInstanceSubsystemDRB,
                    Log,
                    TEXT("LeaveLobby completato.")
                );

                ActiveLobbyId = FLobbyId();
            });
}

void UEosGameInstanceSubsystemDRB::FinalizeSuccessfulLogin(FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	RegisterLocalOnlineUser(PlatformUserId);

	// Aggancio External UI (Overlay Epic)
	if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
	{
		OnlineServicesInfoInternal->ExternalUIEventHandle.Unbind();
		OnlineServicesInfoInternal->ExternalUIEventHandle =
			OnlineServicesInfoInternal->ExternalUIInterface->OnExternalUIStatusChanged().Add(
				[this](const FExternalUIStatusChanged& EventParams)
				{
					HandleExternalUIStatusChanged(EventParams);
				});
	}

	// Aggancio Inviti Lobby
	if (OnlineServicesInfoInternal->LobbiesInterface.IsValid())
	{
		OnlineServicesInfoInternal->LobbyInviteHandle.Unbind();
		OnlineServicesInfoInternal->LobbyInviteHandle = OnlineServicesInfoInternal->LobbiesInterface->OnUILobbyJoinRequested().Add(
			[this](const FUILobbyJoinRequested& InviteData)
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione Lobby ricevuta dall'Overlay."));
				
				if (!InviteData.Result.IsOk())
				{
					//const FOnlineError ErrorResult = InviteData.Result.GetErrorValue();

					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore nella richiesta di unione dalla UI: %s"), *InviteData.Result.GetErrorValue().GetLogString());
					return;
				}
				const TSharedRef<const FLobby> TargetLobby = InviteData.Result.GetOkValue();
				const FLobbyId LobbyIdToJoin = TargetLobby->LobbyId;
				if (UWorld* World = GetWorld())
				{
					if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
					{
						UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Invito accettato. LobbyId: %s"), *ToLogString(LobbyIdToJoin));
						JoinEpicLobby(LobbyIdToJoin, PlayerController);
					}
				}
			});
	}

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Autenticazione finalizzata con successo. OnlineUser registrato e servizi disponibili."));
	OnEpicLoginComplete.Broadcast(true);
}


void UEosGameInstanceSubsystemDRB::ShowEpicLoginUI(
	FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	if (!OnlineServicesInfoInternal)
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("ShowEpicLoginUI: OnlineServicesInfoInternal nullo.")
		);

		OnEpicLoginComplete.Broadcast(false);
		return;
	}

	if (!OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("ShowEpicLoginUI: ExternalUIInterface non disponibile.")
		);

		OnEpicLoginComplete.Broadcast(false);
		return;
	}

	FExternalUIShowLoginUI::Params Params;

	Params.PlatformUserId =
		PlatformUserId;

	UE_LOG(
		LogEosGameInstanceSubsystemDRB,
		Log,
		TEXT("EOS Login: apertura Account Portal tramite ExternalUI.")
	);

	OnlineServicesInfoInternal->ExternalUIInterface
		->ShowLoginUI(MoveTemp(Params))
		.OnComplete(
			[this, PlatformUserId](
				const TOnlineResult<FExternalUIShowLoginUI>& Result)
			{
				if (!Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("ShowLoginUI fallita: %s"),
						*Result.GetErrorValue().GetLogString()
					);

					OnEpicLoginComplete.Broadcast(false);

					return;
				}

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Log,
					TEXT("ShowLoginUI completata. Verifico stato autenticazione...")
				);

				// Non assumiamo che "UI chiusa" == "login riuscito".
				if (!OnlineServicesInfoInternal->AuthInterface->IsLoggedIn(
						PlatformUserId))
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Warning,
						TEXT("ShowLoginUI terminata ma l'utente non risulta autenticato.")
					);

					OnEpicLoginComplete.Broadcast(false);

					return;
				}

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Log,
					TEXT("AccountPortal login riuscito.")
				);

				FinalizeSuccessfulLogin(
					PlatformUserId);
			}
		);
}




int32 UEosGameInstanceSubsystemDRB::GetActiveLobbyMemberCount() const
{
	if (!bHasActiveLobby ||
		!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !CachedActiveLobby.IsValid())
	{
		return 0;
	}

	//using namespace UE::Online;
	//
	/*
	 * Qui non abbiamo una cache locale del FLobby.
	 * Quindi, per il momento il valore va mantenuto
	 * tramite l'ultimo FLobby ricevuto dagli eventi.
	 *
	 * La soluzione migliore sarà aggiungere una
	 * CachedActiveLobby.
	 */
	//
	//return 0;
	
	return CachedActiveLobby->Members.Num();
}

bool UEosGameInstanceSubsystemDRB::IsEpicLobbyLeader(
	APlayerController* PlayerController
) const
{
	if (!PlayerController ||
		!PlayerController->GetLocalPlayer() ||
		!CachedActiveLobby.IsValid())
	{
		return false;
	}

	const FPlatformUserId PlatformUserId =
		PlayerController->GetLocalPlayer()->GetPlatformUserId();

	const UOnlineUserInfo* UserInfo =
		OnlineUserInfos.FindRef(PlatformUserId);

	if (!UserInfo)
	{
		return false;
	}

	return CachedActiveLobby->OwnerAccountId ==
		UserInfo->AccountId;
}


/*
// ============================================================
// READY / UNREADY STATUS
// ============================================================
void UEosGameInstanceSubsystemDRB::SetReadyStatus(APlayerController* PlayerController, bool bIsReady)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer() || !OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !ActiveLobbyId.IsValid()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();
	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FModifyLobbyMemberAttributes::Params ModifyParams;
	ModifyParams.LocalAccountId = UserInfo->AccountId;
	ModifyParams.LobbyId = ActiveLobbyId;

	// MutatedAttributes è la variabile corretta per OSSv2
	ModifyParams.MutatedAttributes.Add(FName(TEXT("IsReady")), bIsReady);

	OnlineServicesInfoInternal->LobbiesInterface->ModifyLobbyMemberAttributes(MoveTemp(ModifyParams))
		.OnComplete([this](const TOnlineResult<FModifyLobbyMemberAttributes>& Result)
		{
			if (!Result.IsOk())
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore aggiornamento Ready Status: %s"), *Result.GetErrorValue().GetLogString());
			}
			else
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Stato Ready inviato con successo al server."));
			}
		});
}

// ============================================================
// ESPULSIONE GIOCATORE (KICK)
// ============================================================
void UEosGameInstanceSubsystemDRB::KickPlayerFromLobby(APlayerController* PlayerController, FString TargetAccountIdStr)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer() || !OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !ActiveLobbyId.IsValid()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();
	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FAccountId TargetAccountId;
	if (TryGetAccountIdFromString(TargetAccountIdStr, TargetAccountId))
	{
		FKickLobbyMember::Params KickParams;
		KickParams.LocalAccountId = UserInfo->AccountId;
		KickParams.LobbyId = ActiveLobbyId;
		KickParams.TargetAccountId = TargetAccountId;

		OnlineServicesInfoInternal->LobbiesInterface->KickLobbyMember(MoveTemp(KickParams))
			.OnComplete([this](const TOnlineResult<FKickLobbyMember>& Result)
			{
				if (!Result.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore Kick: %s"), *Result.GetErrorValue().GetLogString());
				}
			});
	}
}

// ============================================================
// PROMUOVI A CAPOSQUADRA
// ============================================================
void UEosGameInstanceSubsystemDRB::PromotePlayerToHost(APlayerController* PlayerController, FString TargetAccountIdStr)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer() || !OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !ActiveLobbyId.IsValid()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();
	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FAccountId TargetAccountId;
	if (TryGetAccountIdFromString(TargetAccountIdStr, TargetAccountId))
	{
		FPromoteLobbyMember::Params PromoteParams;
		PromoteParams.LocalAccountId = UserInfo->AccountId;
		PromoteParams.LobbyId = ActiveLobbyId;
		PromoteParams.TargetAccountId = TargetAccountId;

		OnlineServicesInfoInternal->LobbiesInterface->PromoteLobbyMember(MoveTemp(PromoteParams))
			.OnComplete([this](const TOnlineResult<FPromoteLobbyMember>& Result)
			{
				if (!Result.IsOk())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore promozione: %s"), *Result.GetErrorValue().GetLogString());
				}
			});
	}
}

// ============================================================
// AVVIO PARTITA
// ============================================================
void UEosGameInstanceSubsystemDRB::StartLobbyMatch(APlayerController* PlayerController)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer() || !OnlineServicesInfoInternal->LobbiesInterface.IsValid() || !ActiveLobbyId.IsValid()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();
	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	FModifyLobbyAttributes::Params ModifyParams;
	ModifyParams.LocalAccountId = UserInfo->AccountId;
	ModifyParams.LobbyId = ActiveLobbyId;
	
	// Modifichiamo i dati globali della Lobby per segnalare l'inizio partita
	ModifyParams.MutatedAttributes.Add(FName(TEXT("bMatchStarted")), true);

	OnlineServicesInfoInternal->LobbiesInterface->ModifyLobbyAttributes(MoveTemp(ModifyParams))
		.OnComplete([this](const TOnlineResult<FModifyLobbyAttributes>& Result)
		{
			if (!Result.IsOk())
			{
				UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore avvio partita: %s"), *Result.GetErrorValue().GetLogString());
			}
		});
}
*/


static FString SchemaVariantToString(
	const UE::Online::FSchemaVariant& Value
)
{
	using namespace UE::Online;

	switch (Value.GetType())
	{
	case ESchemaAttributeType::String:
		return Value.GetString();

	case ESchemaAttributeType::Int64:
		return FString::Printf(
			TEXT("%lld"),
			Value.GetInt64()
		);

	case ESchemaAttributeType::Double:
		return FString::SanitizeFloat(
			Value.GetDouble()
		);

	case ESchemaAttributeType::Bool:
		return Value.GetBoolean()
			? TEXT("true")
			: TEXT("false");

	default:
		return FString();
	}
}


void UEosGameInstanceSubsystemDRB::BindLobbyEvents()
{
    if (!OnlineServicesInfoInternal)
    {
        return;
    }

    if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Error,
            TEXT("Cannot bind lobby events: LobbiesInterface invalid")
        );

        return;
    }

    auto& Lobbies = OnlineServicesInfoInternal->LobbiesInterface;


    // ------------------------------------------------------------------------
    // Lobby Joined
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyJoinedHandle.Unbind();

    OnlineServicesInfoInternal->LobbyJoinedHandle =
        Lobbies->OnLobbyJoined().Add(
            [this](const UE::Online::FLobbyJoined& EventParams)
            {
                HandleLobbyJoined(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Lobby Left
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyLeftHandle.Unbind();

    OnlineServicesInfoInternal->LobbyLeftHandle =
        Lobbies->OnLobbyLeft().Add(
            [this](const UE::Online::FLobbyLeft& EventParams)
            {
                HandleLobbyLeft(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Member Joined
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyMemberJoinedHandle.Unbind();

    OnlineServicesInfoInternal->LobbyMemberJoinedHandle =
        Lobbies->OnLobbyMemberJoined().Add(
            [this](const UE::Online::FLobbyMemberJoined& EventParams)
            {
                HandleLobbyMemberJoined(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Member Left
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyMemberLeftHandle.Unbind();

    OnlineServicesInfoInternal->LobbyMemberLeftHandle =
        Lobbies->OnLobbyMemberLeft().Add(
            [this](const UE::Online::FLobbyMemberLeft& EventParams)
            {
                HandleLobbyMemberLeft(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Leader Changed
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyLeaderChangedHandle.Unbind();

    OnlineServicesInfoInternal->LobbyLeaderChangedHandle =
        Lobbies->OnLobbyLeaderChanged().Add(
            [this](const UE::Online::FLobbyLeaderChanged& EventParams)
            {
                HandleLobbyLeaderChanged(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Member Attributes Changed
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyMemberAttributesChangedHandle.Unbind();

    OnlineServicesInfoInternal->LobbyMemberAttributesChangedHandle =
        Lobbies->OnLobbyMemberAttributesChanged().Add(
            [this](const UE::Online::FLobbyMemberAttributesChanged& EventParams)
            {
                HandleLobbyMemberAttributesChanged(EventParams);
            }
        );


    // ------------------------------------------------------------------------
    // Lobby Attributes Changed
    // ------------------------------------------------------------------------

    OnlineServicesInfoInternal->LobbyAttributesChangedHandle.Unbind();

    OnlineServicesInfoInternal->LobbyAttributesChangedHandle =
        Lobbies->OnLobbyAttributesChanged().Add(
            [this](const UE::Online::FLobbyAttributesChanged& EventParams)
            {
                HandleLobbyAttributesChanged(EventParams);
            }
        );
}


void UEosGameInstanceSubsystemDRB::UnbindLobbyEvents()
{
	if (!OnlineServicesInfoInternal)
	{
		return;
	}

	OnlineServicesInfoInternal->LobbyJoinedHandle.Unbind();
	OnlineServicesInfoInternal->LobbyLeftHandle.Unbind();
	OnlineServicesInfoInternal->LobbyMemberJoinedHandle.Unbind();
	OnlineServicesInfoInternal->LobbyMemberLeftHandle.Unbind();
	OnlineServicesInfoInternal->LobbyLeaderChangedHandle.Unbind();
	OnlineServicesInfoInternal->LobbyMemberAttributesChangedHandle.Unbind();
	OnlineServicesInfoInternal->LobbyAttributesChangedHandle.Unbind();
}



void UEosGameInstanceSubsystemDRB::HandleLobbyJoined(
	const UE::Online::FLobbyJoined& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;

	ActiveLobbyId =
		EventParams.Lobby->LobbyId;

	bHasActiveLobby = true;

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("LobbyJoined: Lobby=%s Members=%d Leader=%s"),
		*UE::Online::ToLogString(EventParams.Lobby->LobbyId),
		EventParams.Lobby->Members.Num(),
		*UE::Online::ToLogString(EventParams.Lobby->OwnerAccountId)
	);
}

void UEosGameInstanceSubsystemDRB::HandleLobbyLeft(
	const UE::Online::FLobbyLeft& EventParams
)
{
	const UE::Online::FLobby& Lobby =
		EventParams.Lobby.Get();

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("LobbyLeft: Lobby=%s"), *UE::Online::ToLogString(Lobby.LobbyId)
	);

	CachedActiveLobby.Reset();

	ActiveLobbyId =
		UE::Online::FLobbyId();

	bHasActiveLobby = false;

	OnEpicLobbyLeft.Broadcast(
		UE::Online::ToLogString(
			Lobby.LobbyId
		)
	);
}

void UEosGameInstanceSubsystemDRB::HandleLobbyMemberJoined(
	const UE::Online::FLobbyMemberJoined& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;

	const FString MemberAccountId =
		UE::Online::ToString(
			EventParams.Member->AccountId
		);

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("LobbyMemberJoined: Account= %s"), *MemberAccountId);

	OnEpicLobbyMemberJoined.Broadcast(
		MemberAccountId
	);
}

void UEosGameInstanceSubsystemDRB::HandleLobbyMemberLeft(
	const UE::Online::FLobbyMemberLeft& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;

	const FString MemberAccountId =
		UE::Online::ToString(
			EventParams.Member->AccountId
		);

	const FString LeaveReason =
		UE::Online::LexToString(
			EventParams.Reason
		);

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("LobbyMemberLeft: Account=%s Reason=%s"), *MemberAccountId, *LeaveReason);

	OnEpicLobbyMemberLeft.Broadcast(MemberAccountId, LeaveReason);
}

void UEosGameInstanceSubsystemDRB::HandleLobbyLeaderChanged(
	const UE::Online::FLobbyLeaderChanged& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;

	const FString NewLeaderAccountId =
		UE::Online::ToString(
			EventParams.Leader->AccountId
		);

	UE_LOG(
		LogEosGameInstanceSubsystemDRB,
		Log,
		TEXT("LobbyLeaderChanged: NewLeader=%s"),
		*NewLeaderAccountId
	);

	OnEpicLobbyLeaderChanged.Broadcast(
		NewLeaderAccountId
	);
}

void UEosGameInstanceSubsystemDRB::HandleLobbyAttributesChanged(
	const UE::Online::FLobbyAttributesChanged& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;


	for (const TPair<
		UE::Online::FSchemaAttributeId,
		UE::Online::FSchemaVariant>& Added :
		EventParams.AddedAttributes)
	{
		const FString AttributeName =
			Added.Key.ToString();

		const FString AttributeValue =
			SchemaVariantToString(
				Added.Value
			);

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT(
				"LobbyAttributeAdded: %s=%s"
			),
			*AttributeName,
			*AttributeValue
		);

		OnEpicLobbyAttributesChanged.Broadcast(
			AttributeName,
			AttributeValue
		);
	}


	for (const TPair<
		UE::Online::FSchemaAttributeId,
		TPair<
			UE::Online::FSchemaVariant,
			UE::Online::FSchemaVariant
		>>& Changed :
		EventParams.ChangedAttributes)
	{
		const FString AttributeName =
			Changed.Key.ToString();

		const FString AttributeValue =
			SchemaVariantToString(
				Changed.Value.Value
			);

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT(
				"LobbyAttributeChanged: %s=%s"
			),
			*AttributeName,
			*AttributeValue
		);

		OnEpicLobbyAttributesChanged.Broadcast(
			AttributeName,
			AttributeValue
		);
	}
}


void UEosGameInstanceSubsystemDRB::HandleLobbyMemberAttributesChanged(
	const UE::Online::FLobbyMemberAttributesChanged& EventParams
)
{
	CachedActiveLobby = EventParams.Lobby;

	const FString MemberAccountId =
		UE::Online::ToString(
			EventParams.Member->AccountId
		);


	for (const TPair<
		UE::Online::FSchemaAttributeId,
		UE::Online::FSchemaVariant>& Added :
		EventParams.AddedAttributes)
	{
		const FString AttributeName =
			Added.Key.ToString();

		const FString AttributeValue =
			SchemaVariantToString(
				Added.Value
			);

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT(
				"LobbyMemberAttributeAdded: Member=%s Attribute=%s Value=%s"
			),
			*MemberAccountId,
			*AttributeName,
			*AttributeValue
		);

		OnEpicLobbyMemberAttributesChanged.Broadcast(
			MemberAccountId,
			AttributeName,
			AttributeValue
		);
	}


	for (const TPair<
		UE::Online::FSchemaAttributeId,
		TPair<
			UE::Online::FSchemaVariant,
			UE::Online::FSchemaVariant
		>>& Changed :
		EventParams.ChangedAttributes)
	{
		const FString AttributeName =
			Changed.Key.ToString();

		const FString AttributeValue =
			SchemaVariantToString(
				Changed.Value.Value
			);

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT(
				"LobbyMemberAttributeChanged: Member=%s Attribute=%s Value=%s"
			),
			*MemberAccountId,
			*AttributeName,
			*AttributeValue
		);

		OnEpicLobbyMemberAttributesChanged.Broadcast(
			MemberAccountId,
			AttributeName,
			AttributeValue
		);
	}
}


bool UEosGameInstanceSubsystemDRB::TryGetAccountIdFromString(const FString& AccountIdString,
	UE::Online::FAccountId& OutAccountId) const
{
	if (AccountIdString.IsEmpty())
	{
		return false;
	}

	OutAccountId =
		UE::Online::FOnlineIdRegistryRegistry::Get().ToAccountId(
			UE::Online::EOnlineServices::Epic,
			AccountIdString
		);

	return OutAccountId.IsValid();
}



void UEosGameInstanceSubsystemDRB::KickEpicLobbyMember(
    APlayerController* PlayerController,
    FString TargetAccountIdString
)
{
    using namespace UE::Online;

    if (!PlayerController ||
        !PlayerController->GetLocalPlayer())
    {
        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    if (!OnlineServicesInfoInternal ||
        !OnlineServicesInfoInternal->LobbiesInterface.IsValid())
    {
        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    if (!bHasActiveLobby)
    {
        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    if (!IsEpicLobbyLeader(PlayerController))
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Warning,
            TEXT("Kick denied: local player is not lobby leader")
        );

        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    const FPlatformUserId PlatformUserId =
        PlayerController->GetLocalPlayer()
            ->GetPlatformUserId();

    TObjectPtr<UOnlineUserInfo> UserInfo =
        GetOnlineUserInfo(PlatformUserId);

    if (!UserInfo)
    {
        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    FAccountId TargetAccountId;

    if (!TryGetAccountIdFromString(
        TargetAccountIdString,
        TargetAccountId))
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Error,
            TEXT("Kick: invalid TargetAccountIdString: %s"),
            *TargetAccountIdString
        );

        OnEpicLobbyKickComplete.Broadcast(false);
        return;
    }

    FKickLobbyMember::Params Params;

    Params.LocalAccountId =
        UserInfo->AccountId;

    Params.LobbyId =
        ActiveLobbyId;

    Params.TargetAccountId =
        TargetAccountId;


    OnlineServicesInfoInternal->LobbiesInterface
        ->KickLobbyMember(MoveTemp(Params))
        .OnComplete(
            [this](
                const TOnlineResult<FKickLobbyMember>& Result
            )
            {
                const bool bSuccess =
                    Result.IsOk();

                if (!bSuccess)
                {
                    UE_LOG(
                        LogEosGameInstanceSubsystemDRB,
                        Error,
                        TEXT("KickLobbyMember failed")
                    );
                }

                OnEpicLobbyKickComplete.Broadcast(
                    bSuccess
                );
            }
        );
}

void UEosGameInstanceSubsystemDRB::PromoteEpicLobbyMember(
    APlayerController* PlayerController,
    FString TargetAccountIdString
)
{
    using namespace UE::Online;

    if (!PlayerController ||
        !PlayerController->GetLocalPlayer())
    {
        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    if (!OnlineServicesInfoInternal ||
        !OnlineServicesInfoInternal->LobbiesInterface.IsValid())
    {
        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    if (!bHasActiveLobby)
    {
        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    if (!IsEpicLobbyLeader(PlayerController))
    {
        UE_LOG(
            LogEosGameInstanceSubsystemDRB,
            Warning,
            TEXT("Promote denied: local player is not lobby leader")
        );

        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    const FPlatformUserId PlatformUserId =
        PlayerController->GetLocalPlayer()
            ->GetPlatformUserId();

    TObjectPtr<UOnlineUserInfo> UserInfo =
        GetOnlineUserInfo(PlatformUserId);

    if (!UserInfo)
    {
        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    FAccountId TargetAccountId;

    if (!TryGetAccountIdFromString(
        TargetAccountIdString,
        TargetAccountId))
    {
        OnEpicLobbyPromoteComplete.Broadcast(false);
        return;
    }

    FPromoteLobbyMember::Params Params;

    Params.LocalAccountId =
        UserInfo->AccountId;

    Params.LobbyId =
        ActiveLobbyId;

    Params.TargetAccountId =
        TargetAccountId;


    OnlineServicesInfoInternal->LobbiesInterface
        ->PromoteLobbyMember(MoveTemp(Params))
        .OnComplete(
            [this](
                const TOnlineResult<FPromoteLobbyMember>& Result
            )
            {
                const bool bSuccess =
                    Result.IsOk();

                if (!bSuccess)
                {
                    UE_LOG(
                        LogEosGameInstanceSubsystemDRB,
                        Error,
                        TEXT("PromoteLobbyMember failed")
                    );
                }

                OnEpicLobbyPromoteComplete.Broadcast(
                    bSuccess
                );
            }
        );
}

void UEosGameInstanceSubsystemDRB::InviteEpicLobbyMember(
	APlayerController* PlayerController,
	FString TargetAccountIdString
)
{
	using namespace UE::Online;

	if (!PlayerController ||
		!PlayerController->GetLocalPlayer())
	{
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
	{
		return;
	}

	if (!bHasActiveLobby)
	{
		return;
	}

	const FPlatformUserId PlatformUserId =
		PlayerController->GetLocalPlayer()
			->GetPlatformUserId();

	TObjectPtr<UOnlineUserInfo> UserInfo =
		GetOnlineUserInfo(PlatformUserId);

	if (!UserInfo)
	{
		return;
	}

	FAccountId TargetAccountId;

	if (!TryGetAccountIdFromString(
		TargetAccountIdString,
		TargetAccountId))
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("Invite: invalid TargetAccountIdString")
		);

		return;
	}

	FInviteLobbyMember::Params Params;

	Params.LocalAccountId =
		UserInfo->AccountId;

	Params.LobbyId =
		ActiveLobbyId;

	Params.TargetAccountId =
		TargetAccountId;


	OnlineServicesInfoInternal->LobbiesInterface
		->InviteLobbyMember(MoveTemp(Params))
		.OnComplete(
			[](
				const TOnlineResult<FInviteLobbyMember>& Result
			)
			{
				if (Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Log,
						TEXT("Lobby invitation sent")
					);
				}
				else
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("Lobby invitation failed")
					);
				}
			}
		);
}

void UEosGameInstanceSubsystemDRB::SetEpicLobbyReady(
	APlayerController* PlayerController,
	bool bReady
)
{
	if (!PlayerController ||
		!PlayerController->GetLocalPlayer())
	{
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
	{
		return;
	}

	if (!bHasActiveLobby)
	{
		return;
	}

	const FPlatformUserId PlatformUserId =
		PlayerController->GetLocalPlayer()->GetPlatformUserId();

	TObjectPtr<UOnlineUserInfo> UserInfo =
		GetOnlineUserInfo(PlatformUserId);

	if (!UserInfo)
	{
		return;
	}

	using namespace UE::Online;

	FModifyLobbyMemberAttributes::Params Params;

	Params.LocalAccountId =
		UserInfo->AccountId;

	Params.LobbyId =
		ActiveLobbyId;

	Params.UpdatedAttributes.Add(
		FSchemaAttributeId(TEXT("bIsReady")),
		FSchemaVariant(bReady)
	);

	OnlineServicesInfoInternal->LobbiesInterface
		->ModifyLobbyMemberAttributes(MoveTemp(Params))
		.OnComplete(
			[](const TOnlineResult<FModifyLobbyMemberAttributes>& Result)
			{
				if (!Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("SetEpicLobbyReady failed")
					);
				}
			}
		);
}


bool UEosGameInstanceSubsystemDRB::IsInEpicLobby() const
{
	return bHasActiveLobby;
}

FString UEosGameInstanceSubsystemDRB::GetActiveLobbyIdString() const
{
	if (!bHasActiveLobby)
	{
		return FString();
	}

	return UE::Online::ToLogString(
		ActiveLobbyId
	);
}

FString UEosGameInstanceSubsystemDRB::GetActiveLobbyName() const
{
	if (!bHasActiveLobby || !CachedActiveLobby.IsValid())
	{
		return FString();
	}

	if (const UE::Online::FSchemaVariant* NameVariant =
		CachedActiveLobby->Attributes.Find(FName(TEXT("LobbyName"))))
	{
		return NameVariant->GetString();
	}

	return FString();
}

/*
FString UEosGameInstanceSubsystemDRB::GetActiveLobbyLeaderDisplayName() const
{
	if (!bHasActiveLobby || !CachedActiveLobby.IsValid())
	{
		return FString();
	}

	const TSharedRef<const UE::Online::FLobbyMember>* LeaderMember =
		CachedActiveLobby->Members.Find(
			CachedActiveLobby->OwnerAccountId
		);

	if (LeaderMember)
	{
		//return (*LeaderMember)->PlatformDisplayName;
		OnlineServicesInfoInternal->UserInfoInterface
	}

	return FString();
}
*/

TArray<FDRBLobbyMemberInfo>
UEosGameInstanceSubsystemDRB::GetActiveLobbyMembers() const
{
	TArray<FDRBLobbyMemberInfo> Result;

	if (!bHasActiveLobby ||
		!CachedActiveLobby.IsValid())
	{
		return Result;
	}

	for (const TPair<
		UE::Online::FAccountId,
		TSharedRef<const UE::Online::FLobbyMember>>& Pair
		: CachedActiveLobby->Members)
	{
		const UE::Online::FAccountId& AccountId =
			Pair.Key;

		const TSharedRef<const UE::Online::FLobbyMember>& Member =
			Pair.Value;

		FDRBLobbyMemberInfo Info;

		Info.AccountId =
			UE::Online::ToString(AccountId);

		if (const FString* DisplayName =
			CachedLobbyMemberDisplayNames.Find(Info.AccountId))
		{
			Info.DisplayName = *DisplayName;
		}
		else
		{
			Info.DisplayName = TEXT("Caricamento...");
		}

		Info.bIsLeader =
			(AccountId == CachedActiveLobby->OwnerAccountId);

		Info.bIsLocalMember =
			Member->bIsLocalMember;

		Result.Add(MoveTemp(Info));
	}

	return Result;
}

/*
void UEosGameInstanceSubsystemDRB::QueryActiveLobbyMemberNicknames(
	APlayerController* PlayerController
)
{
	using namespace UE::Online;

	if (!PlayerController ||
		!PlayerController->GetLocalPlayer())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	if (!bHasActiveLobby ||
		!CachedActiveLobby.IsValid())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->UserInfoInterface.IsValid())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	const FPlatformUserId PlatformUserId =
		PlayerController->GetLocalPlayer()->GetPlatformUserId();

	TObjectPtr<UOnlineUserInfo> LocalUserInfo =
		GetOnlineUserInfo(PlatformUserId);

	if (!LocalUserInfo)
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	const FAccountId LocalAccountId =
		LocalUserInfo->AccountId;

	TArray<FAccountId> AccountIds;

	for (const TPair<
		FAccountId,
		TSharedRef<const FLobbyMember>>& Pair
		: CachedActiveLobby->Members)
	{
		AccountIds.Add(Pair.Key);
	}

	if (AccountIds.Num() == 0)
	{
		CachedLobbyMemberDisplayNames.Empty();

		OnEpicLobbyMemberNicknamesReady.Broadcast(true);
		return;
	}

	FQueryUserInfo::Params QueryParams;

	QueryParams.LocalAccountId = LocalAccountId;
	QueryParams.AccountIds = AccountIds;

	OnlineServicesInfoInternal->UserInfoInterface
		->QueryUserInfo(MoveTemp(QueryParams))
		.OnComplete(
			[this, LocalAccountId, AccountIds](
				const TOnlineResult<FQueryUserInfo>& Result
			)
			{
				if (!Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("QueryUserInfo fallita: %s"),
						*Result.GetErrorValue().GetLogString()
					);

					OnEpicLobbyMemberNicknamesReady.Broadcast(false);
					return;
				}

				CachedLobbyMemberDisplayNames.Empty();

				for (const FAccountId& AccountId : AccountIds)
				{
					FGetUserInfo::Params GetParams;

					GetParams.LocalAccountId = LocalAccountId;
					GetParams.AccountId = AccountId;

					TOnlineResult<FGetUserInfo> UserInfoResult =
						OnlineServicesInfoInternal->UserInfoInterface
							->GetUserInfo(MoveTemp(GetParams));

					if (!UserInfoResult.IsOk())
					{
						UE_LOG(
							LogEosGameInstanceSubsystemDRB,
							Warning,
							TEXT("Impossibile ottenere UserInfo per AccountId=%s"),
							*ToLogString(AccountId)
						);

						continue;
					}

					const TSharedRef<FUserInfo>& UserInfo =
						UserInfoResult.GetOkValue().UserInfo;

					CachedLobbyMemberDisplayNames.Add(
						ToString(AccountId),
						UserInfo->DisplayName
					);
				}

				OnEpicLobbyMemberNicknamesReady.Broadcast(true);
			}
		);
}
*/

void UEosGameInstanceSubsystemDRB::QueryActiveLobbyMemberNicknames(
	APlayerController* PlayerController
)
{
	using namespace UE::Online;

	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	if (!bHasActiveLobby || !CachedActiveLobby.IsValid())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->UserInfoInterface.IsValid())
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	// ---------------------------------------------------------
	// 1. Recuperiamo l'utente locale
	// ---------------------------------------------------------

	const FPlatformUserId PlatformUserId =
		PlayerController->GetLocalPlayer()->GetPlatformUserId();

	TObjectPtr<UOnlineUserInfo> LocalUserInfo =
		GetOnlineUserInfo(PlatformUserId);

	if (!LocalUserInfo)
	{
		OnEpicLobbyMemberNicknamesReady.Broadcast(false);
		return;
	}

	const FAccountId LocalAccountId =
		LocalUserInfo->AccountId;

	// ---------------------------------------------------------
	// 2. Puliamo la cache
	// ---------------------------------------------------------

	CachedLobbyMemberDisplayNames.Empty();

	// ---------------------------------------------------------
	// 3. Salviamo subito il DisplayName del giocatore locale
	// ---------------------------------------------------------

	const FString LocalDisplayName =
		GetUserDisplayName(PlayerController);

	CachedLobbyMemberDisplayNames.Add(
		ToString(LocalAccountId),
		LocalDisplayName
	);

	// ---------------------------------------------------------
	// 4. Prepariamo gli AccountId dei membri REMOTI
	// ---------------------------------------------------------

	TArray<FAccountId> RemoteAccountIds;

	for (const TPair<
		FAccountId,
		TSharedRef<const FLobbyMember>>& Pair
		: CachedActiveLobby->Members)
	{
		const FAccountId& MemberAccountId = Pair.Key;

		if (MemberAccountId == LocalAccountId)
		{
			continue;
		}

		RemoteAccountIds.Add(MemberAccountId);
	}

	// ---------------------------------------------------------
	// 5. Se non ci sono altri giocatori, abbiamo già finito
	// ---------------------------------------------------------

	if (RemoteAccountIds.Num() == 0)
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT("Lobby: nessun membro remoto da interrogare.")
		);

		OnEpicLobbyMemberNicknamesReady.Broadcast(true);
		return;
	}

	// ---------------------------------------------------------
	// 6. Query degli utenti remoti
	// ---------------------------------------------------------

	FQueryUserInfo::Params QueryParams;

	QueryParams.LocalAccountId = LocalAccountId;
	QueryParams.AccountIds = RemoteAccountIds;

	OnlineServicesInfoInternal->UserInfoInterface
		->QueryUserInfo(MoveTemp(QueryParams))
		.OnComplete(
			[this, LocalAccountId, RemoteAccountIds](
				const TOnlineResult<FQueryUserInfo>& Result
			)
			{
				if (!Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("QueryUserInfo fallita: %s"),
						*Result.GetErrorValue().GetLogString()
					);

					OnEpicLobbyMemberNicknamesReady.Broadcast(false);
					return;
				}

				// -------------------------------------------------
				// 7. Recuperiamo il DisplayName di ogni utente
				// -------------------------------------------------

				for (const FAccountId& AccountId : RemoteAccountIds)
				{
					FGetUserInfo::Params GetParams;

					GetParams.LocalAccountId = LocalAccountId;
					GetParams.AccountId = AccountId;

					TOnlineResult<FGetUserInfo> UserInfoResult =
						OnlineServicesInfoInternal->UserInfoInterface
							->GetUserInfo(MoveTemp(GetParams));

					if (!UserInfoResult.IsOk())
					{
						UE_LOG(
							LogEosGameInstanceSubsystemDRB,
							Warning,
							TEXT("Impossibile ottenere UserInfo per AccountId: %s"),
							*ToString(AccountId)
						);

						continue;
					}

					const TSharedRef<FUserInfo>& UserInfo =
						UserInfoResult.GetOkValue().UserInfo;

					CachedLobbyMemberDisplayNames.Add(
						ToString(AccountId),
						UserInfo->DisplayName
					);

					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Log,
						TEXT("Nickname recuperato: %s -> %s"),
						*ToString(AccountId),
						*UserInfo->DisplayName
					);
				}

				// -------------------------------------------------
				// 8. Avvisiamo il Blueprint
				// -------------------------------------------------

				OnEpicLobbyMemberNicknamesReady.Broadcast(true);
			}
		);
}

FString UEosGameInstanceSubsystemDRB::GetCachedLobbyMemberDisplayName(
	FString MemberAccountId
) const
{
	if (const FString* DisplayName =
		CachedLobbyMemberDisplayNames.Find(MemberAccountId))
	{
		return *DisplayName;
	}

	return FString();
}

FString UEosGameInstanceSubsystemDRB::GetLobbyMemberAccountIdFromDisplayName(
	FString DisplayName
) const
{
	for (const TPair<FString, FString>& Pair : CachedLobbyMemberDisplayNames)
	{
		if (Pair.Value.Equals(DisplayName, ESearchCase::IgnoreCase))
		{
			return Pair.Key;
		}
	}

	return FString();
}


///^^^ DRB - LOBBY ^^^///

/// <summary>
/// Constructor for UOnlineUserInfo object
/// </summary>
UOnlineUserInfo::UOnlineUserInfo()
{

}

/// <summary>
/// Return debug string for UOnlineUserInfo
/// </summary>
/// <returns>String representation of UOnlineUserInfo</returns>
const FString UOnlineUserInfo::DebugInfoToString()
{
	int32 UserIndex = this->LocalUserIndex;
	int32 PlatformId = this->PlatformUserId;
	TArray<FStringFormatArg> FormatArgs;
	FormatArgs.Add(FStringFormatArg(UserIndex));
	FormatArgs.Add(FStringFormatArg(PlatformId));
	return FString::Format(TEXT("LocalUserNumber: {0}, PlatformUserId: {1}"), FormatArgs);
}