#pragma once

#include "CoreMinimal.h"
///--- DRB - ACCOUNT ---///
#include "Online/Auth.h"

#include "Online/OnlineServices.h"
#include "Online/Social.h"
#include "Online/Presence.h"
#include "Online/UserInfo.h"
#include "Online/ExternalUI.h" // per includere l'account overlay
///^^^ DRB - ACCOUNT ^^^///
///
///--- DRB - LOBBY ---///
#include "Online/Lobbies.h"
#include "Online/Sessions.h"
///^^^ DRB - LOBBY ^^^///
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/TitleFile.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "EosGameInstanceSubsystemDRB.generated.h"

////////////////////////////////////////////////////////
/// LOG

DECLARE_LOG_CATEGORY_EXTERN(LogEosGameInstanceSubsystemDRB, Log, All);

////////////////////////////////////////////////////////
/// DELEGATE

	///--- DRB - ACCOUNT ---///
	// Delegates for online actions status in game
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLoginCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLogoutCompleteDelegate, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExternalUIChangeDelegate, bool, bIsOverlayActive);

	///^^^ DRB - ACCOUNT ^^^///
	///
	///--- DRB - LOBBY ---///
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyCreateCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyDestroyCompleteDelegate, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyJoinCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyLeaveCompleteDelegate, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbiesSearchCompleteDelegate, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyMemberLeftDelegate,	FString, MemberAccountId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyLeaderChangedDelegate, FString, NewLeaderAccountId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpicLobbyMemberJoinedDelegate, FString, MemberAccountId);

	///^^^ DRB - LOBBY ^^^///

/**
 *
 */
UCLASS(BlueprintType)
class HORRORDRB5_8_API UEosGameInstanceSubsystemDRB : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	///--- DRB - ACCOUNT ---///
	// Event Dispatcher esposto ai Blueprint
	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLoginCompleteDelegate OnEpicLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLogoutCompleteDelegate OnEpicLogoutComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnExternalUIChangeDelegate OnExternalUIChange;
	///^^^ DRB - ACCOUNT ^^^///
	///
	///--- DRB - LOBBY ---///
	// Esponiamo il delegato ai Blueprint
	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLobbyCreateCompleteDelegate OnEpicLobbyCreateComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLobbyDestroyCompleteDelegate OnEpicLobbyDestroyComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLobbyJoinCompleteDelegate OnEpicLobbyJoinComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Events")
	FOnEpicLobbiesSearchCompleteDelegate OnEpicLobbiesSearchComplete;

	// Funzione da chiamare dal bottone "Crea Lobby"
	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void CreateEpicLobby(APlayerController* PlayerController, FString CustomLobbyName, int32 MaxPlayers, int32 PrivacyType);

	// Funzione per uscire/distruggere
	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void DestroyEpicLobby(APlayerController* PlayerController);

	// Funzione nativa chiamata dall'Overlay
	void JoinEpicLobby(UE::Online::FLobbyId LobbyToJoin, APlayerController* PlayerController);

	// Avvia la ricerca sui server Epic
	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void FindEpicLobbies(APlayerController* PlayerController);
	
	//permette di lasciare la lobby
	UFUNCTION(BlueprintCallable, Category="EOS|Lobby")
	void LeaveEpicLobby(APlayerController* PlayerController);

	// Funzioni Helper per la UI (Leggono dall'array salvato in memoria)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS|Lobby")
	int32 GetFoundLobbiesCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS|Lobby")
	FString GetFoundLobbyName(int32 Index) const;

	// Unisciti a una lobby specifica usando l'indice della UI
	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
	void JoinEpicLobbyByIndex(APlayerController* PlayerController, int32 Index);
	
	///^^^ DRB - LOBBY ^^^///

	////////////////////////////////////////////////////////
	/// OnlineSampleOnlineSubsystem Init/Deinit 

	/** Called to determine whether the Subsystem should be created */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Called to initialize Game Instance Subsystem */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called to deinitialize Game Instance Subsystem */
	virtual void Deinitialize() override;

	/** Called to register local online user with the Game Instance Subsystem */
	void RegisterLocalOnlineUser(FPlatformUserId PlatformUserId);

	/** Called to retrieve online user info for this platform user id */
	TObjectPtr<UOnlineUserInfo> GetOnlineUserInfo(FPlatformUserId PlatformUserId);

	/** Called to read the Game's Title File from the backend services and return the contents */
	FString ReadTitleFile(FString Filename, FPlatformUserId PlatformUserId);

protected:
	///--- DRB - LOBBY ---///
	// Memorizziamo l'ID della lobby in cui ci troviamo per poterla chiudere
	UE::Online::FLobbyId ActiveLobbyId;
	// Array in cui salveremo i risultati dell'ultima ricerca
	TArray<TSharedRef<const UE::Online::FLobby>> CurrentSearchResults;
	///^^^ DRB - LOBBY ^^^///
	struct FOnlineServicesInfo
	{
		/** Online Services Pointer - Access Interfaces through this pointer */
		UE::Online::IOnlineServicesPtr OnlineServices = nullptr;

		/** Interface pointers */
		UE::Online::IAuthPtr AuthInterface = nullptr;
		UE::Online::ITitleFilePtr TitleFileInterface = nullptr;

		/** Online Services Implementation */
		UE::Online::EOnlineServices OnlineServicesType = UE::Online::EOnlineServices::Epic; //Null

		/** Title File content */
		UE::Online::FTitleFileContents TitleFileContent;

		///--- DRB - ACCOUNT ---///
		UE::Online::IUserInfoPtr UserInfoInterface = nullptr;
		UE::Online::ISocialPtr SocialInterface = nullptr;
		UE::Online::IPresencePtr PresenceInterface = nullptr;
		UE::Online::IExternalUIPtr ExternalUIInterface = nullptr; // interfaccia UI account overlay
		UE::Online::FOnlineEventDelegateHandle ExternalUIEventHandle; //evento overlay //ANCHE PER LOBBY
		///^^^ DRB - ACCOUNT ^^^///
		///
		///--- DRB - LOBBY ---///
		// Interfaccia per le Lobbies
		UE::Online::ILobbiesPtr LobbiesInterface = nullptr;
		UE::Online::FOnlineEventDelegateHandle LobbyInviteHandle; //per raccogliere gli inviti
		///^^^ DRB - LOBBY ^^^///
		///
		///--- DRB - SESSION ---///
		UE::Online::ISessionsPtr SessionsInterface = nullptr;
		///^^^ DRB - SESSION ^^^///

		/** Reset struct to initial settings */
		void Reset()
		{
			OnlineServices.Reset();
			AuthInterface.Reset();
			TitleFileInterface.Reset();
			///--- DRB - ACCOUNT ---///
			UserInfoInterface.Reset();
			SocialInterface.Reset();
			PresenceInterface.Reset();
			ExternalUIInterface.Reset();
			ExternalUIEventHandle = UE::Online::FOnlineEventDelegateHandle();//Resettiamo anche l'handle //ANCHE PER LOBBY
			///^^^ DRB - ACCOUNT ^^^///
			///
			///--- DRB - LOBBY ---///
			// Reset dell'interfaccia Per le Lobbies
			LobbiesInterface.Reset();
			///^^^ DRB - LOBBY ^^^///
			///
			///--- DRB - SESSION ---///
			SessionsInterface.Reset();
			///^^^ DRB - SESSION ^^^///

			OnlineServicesType = UE::Online::EOnlineServices::Epic; //Null

		}
	};
	////////////////////////////////////////////////////////
	/// Online Services Init

	/** Pointer to an internal struct containing relevant online services pointers */
	FOnlineServicesInfo* OnlineServicesInfoInternal = nullptr;

	/** Called to initialize online services and interface pointers */
	void InitializeOnlineServices();

	////////////////////////////////////////////////////////
	/// Title File

	/** Called to retrieve title file from online services */
	void RetrieveTitleFile(FString Filename, FPlatformUserId PlatformUserId);

	////////////////////////////////////////////////////////
	/// Events

	/** Called to handle the EnumerateFiles async event */
	void HandleEnumerateFiles(const UE::Online::TOnlineResult<UE::Online::FTitleFileEnumerateFiles>& EnumerateFilesResult, TObjectPtr<UOnlineUserInfo> OnlineUser, FString Filename);

	/** Called to handle the ReadFile async event */
	void HandleReadFile(const UE::Online::TOnlineResult<UE::Online::FTitleFileReadFile>& ReadFileResult, FString Filename);

	////////////////////////////////////////////////////////
	/// Online User Information

	/** Called to create a UOnlineUserInfo object for this user */
	TObjectPtr<UOnlineUserInfo> CreateOnlineUserInfo(int32 LocalUserIndex, FPlatformUserId PlatformUserId, UE::Online::FAccountId AccountId, UE::Online::EOnlineServices Services);

	/** Called to register the user with the OnlineUserInfos map and add user after creation with CreateOnlineUserInfo */
	TObjectPtr<UOnlineUserInfo> CreateAndRegisterUserInfo(int32 LocalUserIndex, FPlatformUserId PlatformUserId, UE::Online::FAccountId AccountId, UE::Online::EOnlineServices Services);

	/** Information about each local user */
	TMap<FPlatformUserId, TObjectPtr<UOnlineUserInfo>> OnlineUserInfos;

	/** Friend the UOnlineUserInfo class to access it */
	friend UOnlineUserInfo;

public:
	///---DRB---///
	/** Avvia il processo di login verso gli Epic Online Services (Vecchia funzione)*/
	//UFUNCTION(BlueprintCallable)
	//void LoginEpicAccount(FPlatformUserId PlatformUserId);

	/** Chiamata dal bottone UI per il Login */
	UFUNCTION(BlueprintCallable, Category = "EOS|Auth")
	void LoginWithEpic(APlayerController* PlayerController, bool bAutoLogin);

	/** Chiamata dal bottone UI per il Logout */
	UFUNCTION(BlueprintCallable, Category = "EOS|Auth")
	void LogoutEpicAccount(APlayerController* PlayerController);

	/** Legge il Nickname (DisplayName) dell'utente registrato */
	UFUNCTION(BlueprintCallable, Category = "EOS|UserInfo")
	FString GetUserDisplayName(APlayerController* PlayerController);

	/** Verifica se si è effettuato l'accesso ad EpicGames tramite l'id dispositivo **/
	//UFUNCTION(BlueprintCallable, Category = "EOS|UserInfo")
	//bool IsLoggedWithEpic(FPlatformUserId& PlatformUserId);

	/** Controlla se l'utente risulta loggato ad Epic Games*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS|Auth")
	bool IsEpicAccountLoggedIn(APlayerController* PlayerController);

	//Garbage collector
	/** Impedisce al Garbage Collector di distruggere gli oggetti non tracciati da UPROPERTY */
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	/** Callback asincrona chiamata quando il login è completato (o fallito) */
	void HandleLoginComplete(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& LoginResult, FPlatformUserId PlatformUserId);
	void HandleExternalUIStatusChanged(const UE::Online::FExternalUIStatusChanged& EventParams);
	///^^^DRB^^^///
};

UCLASS()
class HORRORDRB5_8_API UOnlineUserInfo : public UObject
{

	GENERATED_BODY()

public:

	UOnlineUserInfo();

	////////////////////////////////////////////////////////
	/// Online User Fields

	int32 LocalUserIndex = -1;
	FPlatformUserId PlatformUserId;
	UE::Online::FAccountId AccountId;
	UE::Online::EOnlineServices Services = UE::Online::EOnlineServices::Epic; //Null

	////////////////////////////////////////////////////////
	/// Online User Logging/Debugging Functions

	/** Called to obtain OnlineUserInfo as a string */
	const FString DebugInfoToString();

	friend UEosGameInstanceSubsystemDRB;
};
