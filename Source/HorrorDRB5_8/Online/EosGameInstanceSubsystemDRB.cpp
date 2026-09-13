#include "EosGameInstanceSubsystemDRB.h"

#include "Online/CoreOnline.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineError.h"
#include "Online/OnlineServices.h"
#include "Online/Auth.h"
#include "Online/TitleFile.h"

// --- DRB LOBBY OSSv1 ---//
//#include "OnlineSubsystem.h"
//#include "Interfaces/OnlineIdentityInterface.h"
// ^^^ DRB LOBBY OSSv1 ^^^//

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
///
/// <summary>
/// Configura i parametri di login e chiama l'interfaccia Auth per autenticare l'utente
/// </summary>
/*
void UEosGameInstanceSubsystemDRB::LoginEpicAccount(FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	// Verifica che l'interfaccia Auth sia valida
	if (!OnlineServicesInfoInternal->AuthInterface.IsValid())
	{
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore: Interfaccia Auth non valida per il Login."));
		return;
	}

	// Prepara i parametri per il login
	FAuthLogin::Params LoginParams;
	LoginParams.PlatformUserId = PlatformUserId;

	// Utilizza l'Account Portal per aprire il browser (o l'overlay) e far inserire email/password all'utente.
	// Se stai testando in locale con il DevAuthTool, puoi cambiare questo valore in LoginCredentialsType::Developer
	LoginParams.CredentialsType = LoginCredentialsType::AccountPortal;

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Inizio login per PlatformUserId: %d..."), PlatformUserId.GetInternalId());

	// Esegue il login asincrono
	OnlineServicesInfoInternal->AuthInterface->Login(MoveTemp(LoginParams))
		.OnComplete(this, &ThisClass::HandleLoginComplete, PlatformUserId);
}
*/

/* pre gpt
/// <summary>
/// Gestisce il risultato dell'operazione di Login
/// </summary>
void UEosGameInstanceSubsystemDRB::HandleLoginComplete(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& LoginResult, FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	/* PRIMA DI IMPLEMENTARE ANCHE OSSv1
	if (LoginResult.IsOk())
	{
		const FAuthLogin::Result& ResultValue = LoginResult.GetOkValue();

		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Login EOS completato con successo! Account ID: %s"),
			*ToLogString(ResultValue.AccountInfo->AccountId));

		// Dopo che il login ha avuto successo, possiamo registrare l'utente nel nostro sistema locale
		RegisterLocalOnlineUser(PlatformUserId);

		// Delegato: Il login ha avuto successo (True)
		OnEpicLoginComplete.Broadcast(true);
	*/
	/*

	if (LoginResult.IsOk())
	{
		const FAuthLogin::Result& ResultValue = LoginResult.GetOkValue();
		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Login EOS (V2) completato con successo! Account ID: %s"), *ToLogString(ResultValue.AccountInfo->AccountId));

		// Dopo che il login ha avuto successo, possiamo registrare l'utente nel nostro sistema locale
		RegisterLocalOnlineUser(PlatformUserId);

		// --- SINCRONIZZAZIONE FRAMEWORK V1 PER IL NETDRIVER ---
		if (IOnlineSubsystem* OSSv1 = IOnlineSubsystem::Get())
		{
			if (IOnlineIdentityPtr IdentityV1 = OSSv1->GetIdentityInterface())
			{
				// Agganciamo un delegato temporaneo per sapere quando la sincronizzazione in V1 è completa
				IdentityV1->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateLambda(
					[this](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
					{
						if (bWasSuccessful)
						{
							UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Sincronizzazione V1 completata. Il NetDriver EOS puo' ora aprire i socket."));
							// Delegato: Il login ha avuto successo (True)
							OnEpicLoginComplete.Broadcast(true);
						}
						else
						{
							UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore sincronizzazione V1: %s"), *Error);
							OnEpicLoginComplete.Broadcast(false);
						}
					}));

				// Parametri di accesso per accedere silenziosamente sfruttando il token EOS appena ottenuto in V2
				FOnlineAccountCredentials Credentials;
				Credentials.Type = TEXT("persistentauth");

				IdentityV1->Login(0, Credentials);
			}
		}
		else
		{
			// Fallback se il framework V1 è disabilitato
			OnEpicLoginComplete.Broadcast(true);
		}

		// AGGANCIAMO L'OVERLAY SOLO ORA CHE SIAMO LOGGATI
		if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
		{
			// Salviamo l'handle dell'overlay per usarlo al logout
			OnlineServicesInfoInternal->ExternalUIEventHandle = OnlineServicesInfoInternal->ExternalUIInterface->OnExternalUIStatusChanged().Add(
				[this](const UE::Online::FExternalUIStatusChanged& EventParams)
				{
					HandleExternalUIStatusChanged(EventParams);
				});
			//
			//OnlineServicesInfoInternal->LobbyInviteHandle = OnlineServicesInfoInternal->LobbiesInterface->OnUILobbyJoinRequested().Add(
			//	[this](const UE::Online::FUILobbyJoinRequested& InviteData)
			//	{
			//		//UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Invito ricevuto per Lobby ID: %s"), *UE::Online::ToLogString(InviteData.LobbyId));
			//		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione tramite Overlay ricevuta con successo!"));
			//		// Qui chiameremo la funzione JoinLobby()
			//	});
			//
			// AGGANCIO PER L'ACCETTAZIONE DEGLI INVITI (Sintassi OSSv2)
			if (OnlineServicesInfoInternal->LobbiesInterface.IsValid())
			{
				OnlineServicesInfoInternal->LobbyInviteHandle = OnlineServicesInfoInternal->LobbiesInterface->OnUILobbyJoinRequested().Add(
					[this](const UE::Online::FUILobbyJoinRequested& InviteData)
					{
						//UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione tramite Overlay per la Lobby: %s"), *UE::Online::ToLogString(InviteData.LobbyId)); //LobbyId
						//UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione tramite Overlay ricevuta con successo!"));
						//UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Overlay: Invito accettato, avvio unione..."));
						// Qui chiameremo la futura funzione: JoinEpicLobby(InviteData.LobbyId);

						UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Overlay: Invito accettato, estrazione dati in corso..."));

						// 1. Verifichiamo che la richiesta dall'Overlay sia valida e non contenga errori
						if (InviteData.Result.IsOk())
						{
							// 2. Estraiamo l'oggetto Lobby completo
							TSharedRef<const UE::Online::FLobby> TargetLobby = InviteData.Result.GetOkValue();

							// 3. Da questo oggetto, leggiamo il LobbyId
							UE::Online::FLobbyId IdToJoin = TargetLobby->LobbyId;

							UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("ID della Lobby estratto: %s. Avvio connessione..."), *UE::Online::ToLogString(IdToJoin));

							// 4. Chiamiamo la tua funzione nativa passando l'ID esatto!
							JoinEpicLobby(IdToJoin);
						}
						else
						{
							// Se l'invito era scaduto o corrotto, Epic ci manda un errore
							UE::Online::FOnlineError ErrorResult = InviteData.Result.GetErrorValue();
							UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore nella richiesta di unione da UI: %s"), *ErrorResult.GetLogString());
						}
					});
			}
		}

		//prova di debug
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("PlatformUserId Login: %s"),PlatformUserId);
		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("PlatformUserId Login: %s"),
			*ToLogString(PlatformUserId));
		UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("PlatformUserId.InternalId Login: %d"), PlatformUserId.GetInternalId());
	}
	else
	{
		UE::Online::FOnlineError ErrorResult = LoginResult.GetErrorValue();
		UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore durante il Login EOS: %s"), *ErrorResult.GetLogString());

		// Delegato: Il login è fallito (False)
		OnEpicLoginComplete.Broadcast(false);
	}
}
*/


void UEosGameInstanceSubsystemDRB::HandleLoginComplete(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& LoginResult, FPlatformUserId PlatformUserId)
{
	using namespace UE::Online;

	if (LoginResult.IsOk())
	{
		const FAuthLogin::Result& ResultValue = LoginResult.GetOkValue();

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT("Login EOS (OSSv2) completato con successo! Account ID: %s"),
			*ToLogString(ResultValue.AccountInfo->AccountId)
		);

		// Registriamo l'utente nel nostro registro locale.
		RegisterLocalOnlineUser(PlatformUserId);

		// Da questo momento in poi NON dipendiamo più da OSSv1.
		// Il login OSSv2 è sufficiente per il nostro layer online.
		OnEpicLoginComplete.Broadcast(true);

		// ============================================================
		// EPIC OVERLAY / INVITI
		// ============================================================

		if (OnlineServicesInfoInternal->ExternalUIInterface.IsValid())
		{
			// Evitiamo di registrare più volte lo stesso delegate.
			//if (OnlineServicesInfoInternal->ExternalUIEventHandle.IsValid())
			//{
			OnlineServicesInfoInternal->ExternalUIEventHandle.Unbind();
			//}

			OnlineServicesInfoInternal->ExternalUIEventHandle =
				OnlineServicesInfoInternal->ExternalUIInterface->OnExternalUIStatusChanged().Add(
					[this](const FExternalUIStatusChanged& EventParams)
					{
						HandleExternalUIStatusChanged(EventParams);
					}
				);
		}

		// ============================================================
		// INVITI ALLA LOBBY
		// ============================================================

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
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Log,
						TEXT("Richiesta di unione Lobby ricevuta dall'Overlay.")
					);

					if (!InviteData.Result.IsOk())
					{
						const FOnlineError ErrorResult =
							InviteData.Result.GetErrorValue();

						UE_LOG(
							LogEosGameInstanceSubsystemDRB,
							Error,
							TEXT("Errore nella richiesta di unione dalla UI: %s"),
							*ErrorResult.GetLogString()
						);

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
						UE_LOG(
							LogEosGameInstanceSubsystemDRB,
							Error,
							TEXT("Overlay Invite: World non disponibile.")
						);

						return;
					}

					APlayerController* PlayerController =
						World->GetFirstPlayerController();

					if (!PlayerController)
					{
						UE_LOG(
							LogEosGameInstanceSubsystemDRB,
							Error,
							TEXT("Overlay Invite: PlayerController non disponibile.")
						);

						return;
					}

					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Log,
						TEXT("Invito accettato. LobbyId: %s"),
						*ToLogString(LobbyIdToJoin)
					);

					JoinEpicLobby(LobbyIdToJoin, PlayerController);
				}
			);
		}

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Log,
			TEXT("Autenticazione OSSv2 pronta. OnlineUser registrato e servizi disponibili.")
		);
	}
	else
	{
		const FOnlineError ErrorResult = LoginResult.GetErrorValue();

		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("Errore durante il Login EOS: %s"),
			*ErrorResult.GetLogString()
		);

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

	/*
	// --------- INIZIO MACRO EDITOR ---------
#if UE_EDITOR
	// Se siamo nell'Editor, usa il DevAuthTool per un login istantaneo e senza browser
	LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::Developer;
	// Il tool di Epic di default gira su localhost alla porta 8081
	LoginParams.CredentialsId = TEXT("localhost:8081"); // CredentialsId è solitamente una semplice stringa per indicare la porta locale
	// Questo è il nome fittizio che sceglierai dentro il DevAuthTool
	LoginParams.CredentialsToken.Set<FString>(FString(TEXT("Piergi_F"))); // CredentialsToken è un TVariant. Usiamo .Set<FString>() per l'assegnazione sicura
#else // Se il gioco è pacchettizzato o in Standalone, usa il portale web vero e proprio
	// Se bAutoLogin è true, tenta di usare il token salvato in locale.
	// Altrimenti apre il portale Epic nel browser/overlay.
	LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::AccountPortal;
#endif
	// --------- FINE MACRO EDITOR ---------
	*/

	/*
	// Rimuoviamo le macro #if UE_EDITOR e controlliamo a runtime il tipo di simulazione
	if (PlayerController->GetWorld()->IsPlayInEditor())
	{
		// 1. Play In Editor (PIE): Usiamo il DevAuthTool per la massima velocità
		LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::Developer;
		LoginParams.CredentialsId = TEXT("localhost:8081");
		LoginParams.CredentialsToken.Set<FString>(FString(TEXT("Piergi_F")));
	}
	else
	{
		// 2. Standalone o Gioco Pacchettizzato: Usiamo l'Account Portal
		LoginParams.CredentialsType = bAutoLogin ? LoginCredentialsType::PersistentAuth : LoginCredentialsType::AccountPortal;
	}
	*/


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

	//prova debug
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("PlatformUserId inizio Logout: %s"),PlatformUserId);
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

	//prova debug
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

					// SGANCIAMO L'OVERLAY
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
					// ID della lobby appena creata
					ActiveLobbyId = ResultValue.Lobby->LobbyId;
					//UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Lobby creata con successo! Lobby ID: %s"), *ToLogString(ResultValue.Lobby->LobbyId));
					UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Lobby creata con successo! Lobby ID: %s"), *ToLogString(ActiveLobbyId));

					// Avvisiamo la UI che tutto è andato bene
					OnEpicLobbyCreateComplete.Broadcast(true);
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
					//ActiveLobbyId = UE::Online::FLobbyId();
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


/* pre gpt
void UEosGameInstanceSubsystemDRB::JoinEpicLobby(UE::Online::FLobbyId LobbyToJoin)
{
	using namespace UE::Online;

	// Assicuriamoci di avere un controller valido (essendo nel Subsystem, usiamo il primo giocatore locale)
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController || !PlayerController->GetLocalPlayer()) return;

	FPlatformUserId PlatformUserId = PlayerController->GetLocalPlayer()->GetPlatformUserId();

	if (!OnlineServicesInfoInternal->LobbiesInterface.IsValid()) return;

	TObjectPtr<UOnlineUserInfo> UserInfo = GetOnlineUserInfo(PlatformUserId);
	if (!UserInfo) return;

	// Parametri di unione
	FJoinLobby::Params JoinParams;
	JoinParams.LocalAccountId = UserInfo->AccountId;
	JoinParams.LobbyId = LobbyToJoin;

	// Anche chi si unisce attiva il proprio Presence nella stanza
	JoinParams.bPresenceEnabled = true;

	UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Richiesta di unione alla Lobby in corso..."));

	OnlineServicesInfoInternal->LobbiesInterface->JoinLobby(MoveTemp(JoinParams))
		.OnComplete([this, PlayerController](const TOnlineResult<FJoinLobby>& Result)
		{
			if (Result.IsOk())
			{
				const FJoinLobby::Result& ResultValue = Result.GetOkValue();

				// Salviamo l'ID della lobby attiva così potremo uscirne in futuro
				ActiveLobbyId = ResultValue.Lobby->LobbyId;

				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Unito con successo alla Lobby Cloud: %s"), *ToLogString(ActiveLobbyId));

				// FASE 2: TELETRASPORTO P2P (Da implementare a breve)
				// FString HostAddress = ...
				// PlayerController->ClientTravel(HostAddress, TRAVEL_Absolute);



				// --- FASE 2: ESTRAZIONE DATI E TELETRASPORTO P2P VIA EOS ---

				// 1. Otteniamo l'ID dell'host sotto forma di stringa diagnostica
				FAccountId HostId = ResultValue.Lobby->OwnerAccountId;
				FString FullLogStr = ToLogString(HostId);
				FString EosIdStr;

				// 2. Parsing per isolare il Product User ID (EOS=[...])
				int32 StartIdx = FullLogStr.Find(TEXT("EOS=["));
				if (StartIdx != INDEX_NONE)
				{
					StartIdx += 5; // Saltiamo la stringa "EOS=[" (5 caratteri)
					int32 EndIdx = FullLogStr.Find(TEXT("]"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIdx);

					if (EndIdx != INDEX_NONE)
					{
						// Estraiamo esattamente i 32 caratteri alfanumerici
						EosIdStr = FullLogStr.Mid(StartIdx, EndIdx - StartIdx);
					}
				}

				// Se l'estrazione fallisce per qualche motivo, mettiamo un log di emergenza
				if (EosIdStr.IsEmpty())
				{
					UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Impossibile estrarre l'EOS ID dalla stringa: %s"), *FullLogStr);
					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				// 3. Costruiamo l'indirizzo formattato per il SocketSubsystemEOS
				//FString ConnectString = FString::Printf(TEXT("eos:%s"), *EosIdStr);
				//FString ConnectString = FString::Printf(TEXT("%s.eos"), *EosIdStr);
				//FString ConnectString = FString::Printf(TEXT("eos://%s"), *EosIdStr);
				//FString ConnectString = FString::Printf(TEXT("%s:7777"), *EosIdStr);
				FString ConnectString = FString::Printf(TEXT("eos://%s/"), *EosIdStr);

				UE_LOG(LogEosGameInstanceSubsystemDRB, Log, TEXT("Avvio teletrasporto P2P verso l'Host: %s"), *ConnectString);

				// 4. Viaggio verso il Listen Server
				PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);




				OnEpicLobbyJoinComplete.Broadcast(true);
			}
			else
			{
				UE::Online::FOnlineError ErrorResult = Result.GetErrorValue();
				UE_LOG(LogEosGameInstanceSubsystemDRB, Error, TEXT("Errore durante l'unione: %s"), *ErrorResult.GetLogString());
				OnEpicLobbyJoinComplete.Broadcast(false);
			}
		});
}
*/

void UEosGameInstanceSubsystemDRB::JoinEpicLobby(UE::Online::FLobbyId LobbyToJoin, APlayerController* PlayerController)
{
	using namespace UE::Online;

	/*
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("JoinEpicLobby: PlayerController non valido.")
		);

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}
	*/

	// ============================================================
	// VALIDAZIONE
	// ============================================================

	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("JoinEpicLobby: PlayerController non valido.")
		);

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}

	if (!OnlineServicesInfoInternal ||
		!OnlineServicesInfoInternal->OnlineServices.IsValid() ||
		!OnlineServicesInfoInternal->LobbiesInterface.IsValid())
	{
		UE_LOG(
			LogEosGameInstanceSubsystemDRB,
			Error,
			TEXT("JoinEpicLobby: Online Services o Lobby Interface non validi.")
		);

		OnEpicLobbyJoinComplete.Broadcast(false);
		return;
	}

	// ============================================================
	// OTTENIAMO L'UTENTE LOCALE
	// ============================================================

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

	// ============================================================
	// JOIN DELLA LOBBY
	// ============================================================

	FJoinLobby::Params JoinParams;
	JoinParams.LocalAccountId = UserInfo->AccountId;
	JoinParams.LobbyId = LobbyToJoin;
	JoinParams.bPresenceEnabled = true;

	UE_LOG(
		LogEosGameInstanceSubsystemDRB,
		Log,
		TEXT("Join Lobby in corso: %s"),
		*ToLogString(LobbyToJoin)
	);

	OnlineServicesInfoInternal->LobbiesInterface
		->JoinLobby(MoveTemp(JoinParams))
		.OnComplete(
			[this, PlayerController, LocalAccountId = UserInfo->AccountId]
			(const TOnlineResult<FJoinLobby>& Result)
			{
				using namespace UE::Online;

				// ====================================================
				// VERIFICA JOIN LOBBY
				// ====================================================

				if (!Result.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("JoinLobby fallita: %s"),
						*Result.GetErrorValue().GetLogString()
					);

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				const FJoinLobby::Result& ResultValue =
					Result.GetOkValue();

				if (!ResultValue.Lobby.IsValid())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("JoinLobby riuscita ma FLobby non è valido.")
					);

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				// Salviamo la Lobby locale.
				ActiveLobbyId = ResultValue.Lobby->LobbyId;

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Log,
					TEXT("Lobby joinata con successo: %s"),
					*ToLogString(ActiveLobbyId)
				);

				// =========================================================
				// RISOLUZIONE DELLA DESTINAZIONE P2P TRAMITE OSSv2
				// =========================================================

				FGetResolvedConnectString::Params ConnectParams;
				ConnectParams.LocalAccountId = LocalAccountId;
				ConnectParams.LobbyId = ActiveLobbyId;
				ConnectParams.PortType = NAME_GamePort;

				IOnlineServicesPtr OnlineServices = UE::Online::GetServices();

				if (!OnlineServices.IsValid())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("Online Services non disponibile.")
					);

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Log,
					TEXT("Richiesta GetResolvedConnectString per Lobby...")
				);

				TOnlineResult<FGetResolvedConnectString> ConnectResult =
					OnlineServicesInfoInternal->OnlineServices
					->GetResolvedConnectString(MoveTemp(ConnectParams));

				if (!ConnectResult.IsOk())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("GetResolvedConnectString fallita: %s"),
						*ConnectResult.GetErrorValue().GetLogString()
					);

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				// ====================================================
				// OTTENIAMO L'URL P2P GENERATO DAL FRAMEWORK
				// ====================================================

				const FString ConnectString =
					ConnectResult.GetOkValue().ResolvedConnectString;

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Warning,
					TEXT("EOS Resolved Connect String = %s"),
					*ConnectString
				);

				if (ConnectString.IsEmpty())
				{
					UE_LOG(
						LogEosGameInstanceSubsystemDRB,
						Error,
						TEXT("Resolved Connect String vuota.")
					);

					OnEpicLobbyJoinComplete.Broadcast(false);
					return;
				}

				// ====================================================
				// CLIENT TRAVEL
				// ====================================================


				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Warning,
					//TEXT("NetMode prima del ClientTravel: %s"),
					//*UEnum::GetValueAsString(GetWorld()->GetNetMode())
					TEXT("NetMode prima del ClientTravel: %d"),
					(int32)GetWorld()->GetNetMode()
				);

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Warning,
					TEXT("World URL prima del ClientTravel: %s"),
					*GetWorld()->URL.ToString()
				);

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Warning,
					TEXT("ClientTravel URL = %s"),
					*ConnectString
				);

				UE_LOG(
					LogEosGameInstanceSubsystemDRB,
					Log,
					TEXT("ClientTravel verso EOS P2P...")
				);

				PlayerController->ClientTravel(
					ConnectString,
					TRAVEL_Absolute
					//TRAVEL_Relative
				);

				// ATTENZIONE:
				// Questo significa che la richiesta di travel è stata
				// avviata, NON che la connessione è già riuscita.
				OnEpicLobbyJoinComplete.Broadcast(true);


				/*
				// UE 5.7 + Online Services OSSv2:
			// il ResolvedConnectString può essere "[EOS:PUID]"
			// ma FURL necessita anche del Map.
			// Per il nostro Hub usiamo il package path completo.
			const FString HubMapPath = TEXT("/Game/FirstPerson/Lvl_FirstPerson");

			const FString ClientTravelURL =
				FString::Printf(
					TEXT("%s%s"),
					*ConnectString,
					*HubMapPath
				);

			UE_LOG(
				LogEosGameInstanceSubsystemDRB,
				Warning,
				TEXT("ClientTravel URL finale: [%s]"),
				*ClientTravelURL
			);

			PlayerController->ClientTravel(
				ClientTravelURL,
				TRAVEL_Absolute
			);

			UE_LOG(
				LogEosGameInstanceSubsystemDRB,
				Log,
				TEXT("ClientTravel verso EOS P2P avviato.")
			);

			OnEpicLobbyJoinComplete.Broadcast(true);
				*/
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