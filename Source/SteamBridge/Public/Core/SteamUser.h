// Copyright 2020 Russ 'trdwll' Treadwell <trdwll.com>. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steam.h"
#include "SteamEnums.h"
#include "SteamStructs.h"
#include "UObject/NoExportTypes.h"

#include "SteamUser.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnClientGameServerDenyDelegate, int32, AppID, FString, GameServerIP, int32, GameServerPort, bool, bSecure, ESteamDenyReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnDurationControlDelegate, ESteamResult, Result, int32, AppId, bool, bApplicable, int32, csecsLast5h, ESteamDurationControlProgress, Progress, ESteamDurationControlNotification, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncryptedAppTicketResponseDelegate, ESteamResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameWebCallbackDelegate, FString, URL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetAuthSessionTicketResponseDelegate, FHAuthTicket, AuthTicket, ESteamResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIPCFailureDelegate, ESteamFailureType, FailureType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLicensesUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMicroTxnAuthorizationResponseDelegate, int32, AppID, FString, OrderID, bool, bAuthorized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamServerConnectFailureDelegate, ESteamResult, Result, bool, bStillRetrying);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSteamServersConnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamServersDisconnectedDelegate, ESteamResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoreAuthURLResponseDelegate, FString, URL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnValidateAuthTicketResponseDelegate, FSteamID, SteamID, ESteamAuthSessionResponse, AuthSessionResponse, FSteamID, OwnerSteamID);

/**
 * Functions for accessing and manipulating Steam user information.
 * https://partner.steamgames.com/doc/api/ISteamUser
 */
UCLASS()
class STEAMBRIDGE_API USteamUser final : public UObject
{
	GENERATED_BODY()

public:
	USteamUser();
	~USteamUser();

	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User", meta = (DisplayName = "Steam User", CompactNodeTitle = "SteamUser"))
	static USteamUser* GetSteamUser() { return USteamUser::StaticClass()->GetDefaultObject<USteamUser>(); }

	/**
	 * Set the rich presence data for an unsecured game server that the user is playing on. This allows friends to be able to view the game info and join your game.
     *
     * When you are using Steam authentication system this call is never required, the auth system automatically sets the appropriate rich presence.
     *
     * @param FSteamID SteamID
     * @param const FString & IP
     * @param int32 Port
     * @return void
     */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void AdvertiseGame(FSteamID SteamID, const FString& IP, int32 Port);

	/**
	 * Authenticate the ticket from the entity Steam ID to be sure it is valid and isn't reused.
     * The ticket is created on the entity with GetAuthSessionTicket or ISteamGameServer::GetAuthSessionTicket and then needs to be provided over the network for the other end to validate.
     * This registers for ValidateAuthTicketResponse_t callbacks if the entity goes offline or cancels the ticket. See EAuthSessionResponse for more information.
     * When the multiplayer session terminates you must call EndAuthSession.
     *
     * @param TArray<uint8> Ticket
     * @param FSteamID SteamID
     * @return ESteamBeginAuthSessionResult
     *
     * Triggers a ValidateAuthTicketResponse_t callback.
     */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	ESteamBeginAuthSessionResult BeginAuthSession(TArray<uint8> Ticket, FSteamID SteamID) { return (ESteamBeginAuthSessionResult)SteamUser()->BeginAuthSession(Ticket.GetData(), Ticket.Num(), SteamID.Value); }

	/**
	 * Checks if the current users looks like they are behind a NAT device.
     * This is only valid if the user is connected to the Steam servers and may not catch all forms of NAT.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BIsBehindNAT() { return SteamUser()->BIsBehindNAT(); }

	/**
     * Checks whether the user's phone number is used to uniquely identify them.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BIsPhoneIdentifying() { return SteamUser()->BIsPhoneIdentifying(); }

	/**
     * Checks whether the current user's phone number is awaiting (re)verification.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BIsPhoneRequiringVerification() { return SteamUser()->BIsPhoneRequiringVerification(); }

	/**
     * Checks whether the current user has verified their phone number.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BIsPhoneVerified() { return SteamUser()->BIsPhoneVerified(); }

	/**
     * Checks whether the current user has Steam Guard two factor authentication enabled on their account.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BIsTwoFactorEnabled() { return SteamUser()->BIsTwoFactorEnabled(); }

	/**
	 * Checks if the current user's Steam client is connected to the Steam servers.
	 * If it's not then no real-time services provided by the Steamworks API will be enabled. The Steam client will automatically be trying to recreate the connection as often as possible.
     * When the connection is restored a SteamServersConnected_t callback will be posted.
	 * You usually don't need to check for this yourself. All of the API calls that rely on this will check internally. Forcefully disabling stuff when the player loses access is usually not a very -
     * good experience for the player and you could be preventing them from accessing APIs that do not need a live connection to Steam.
     *
     * @return bool
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool BLoggedOn() { return SteamUser()->BLoggedOn(); }

	/**
     * Cancels an auth ticket received from GetAuthSessionTicket. This should be called when no longer playing with the specified entity.
     *
     * @param FHAuthTicket AuthTicket
     * @return void
     */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void CancelAuthTicket(FHAuthTicket AuthTicket) { SteamUser()->CancelAuthTicket(AuthTicket); }

	/**
     * Decodes the compressed voice data returned by GetVoice.
     * The output data is raw single-channel 16-bit PCM audio. The decoder supports any sample rate from 11025 to 48000. See GetVoiceOptimalSampleRate for more information.
     * It is recommended that you start with a 20KiB buffer and then reallocate as necessary.
     *
     * @param TArray<uint8> CompressedBuffer
     * @param TArray<uint8> & UncompressedBuffer
     * @return ESteamVoiceResult
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	ESteamVoiceResult DecompressVoice(TArray<uint8> CompressedBuffer, TArray<uint8>& UncompressedBuffer);

	/**
     * Ends an auth session that was started with BeginAuthSession. This should be called when no longer playing with the specified entity.
     *
     * @param FSteamID SteamID
     * @return void
     */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void EndAuthSession(FSteamID SteamID) { SteamUser()->EndAuthSession(SteamID.Value); }

	/**
	 * Retrieve a authentication ticket to be sent to the entity who wishes to authenticate you.
	 * After calling this you can send the ticket to the entity where they can then call BeginAuthSession/ISteamGameServer::BeginAuthSession to verify this entities integrity.
	 * When creating a ticket for use by the ISteamUserAuth/AuthenticateUserTicket Web API, the calling application should wait for the GetAuthSessionTicketResponse_t callback generated by the API -
     * call before attempting to use the ticket to ensure that the ticket has been communicated to the server. If this callback does not come in a timely fashion (10 - 20 seconds), then your client -
     * is not connected to Steam, and the AuthenticateUserTicket call will fail because it can not authenticate the user.
     *
     * @param TArray<uint8> & Ticket
     * @return FHAuthTicket
	 *
	 * Triggers a GetAuthSessionTicketResponse_t callback.
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	FHAuthTicket GetAuthSessionTicket(TArray<uint8>& Ticket);

	/**
     * Checks to see if there is captured audio data available from GetVoice, and gets the size of the data.
	 * Most applications will only use compressed data and should ignore the other parameters, which exist primarily for backwards compatibility. See GetVoice for further explanation of "uncompressed" data.
	 *
     * @param int32 & CompressedSize
     * @return ESteamVoiceResult
     */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	ESteamVoiceResult GetAvailableVoice(int32& CompressedSize) { return (ESteamVoiceResult)SteamUser()->GetAvailableVoice((uint32*)&CompressedSize); }

	/**
	 * Retrieves anti indulgence / duration control for current user / game combination.
	 *
	 * @return FSteamAPICall
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	FSteamAPICall GetDurationControl() { return (FSteamAPICall)SteamUser()->GetDurationControl(); }

	/**
	 * Retrieve an encrypted ticket.
	 * This should be called after requesting an encrypted app ticket with RequestEncryptedAppTicket and receiving the EncryptedAppTicketResponse_t call result.
	 *
	 * If you call this without calling RequestEncryptedAppTicket, the call may succeed but you will likely get a stale ticket.
	 *
	 * @param TArray<uint8> & Ticket
	 * @return bool
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	bool GetEncryptedAppTicket(TArray<uint8>& Ticket);

	/**
	 * Gets the level of the users Steam badge for your game.
	 * The user can have two different badges for a series; the regular badge (max level 5) and the foil badge (max level 1).
	 *
	 * @param int32 nSeries
	 * @param bool bFoil
	 * @return int32
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	int32 GetGameBadgeLevel(int32 nSeries, bool bFoil) { return SteamUser()->GetGameBadgeLevel(nSeries, bFoil); }

	/**
	 * Gets Steam user handle that this interface represents.
	 * This is only used internally by the API, and by a few select interfaces that support multi-user.
	 *
	 * @return FHSteamUser
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	FHSteamUser GetHSteamUser() { return (FHSteamUser)SteamUser()->GetHSteamUser(); }

	/**
	 * Gets the Steam level of the user, as shown on their Steam community profile.
	 *
	 * @return int32
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	int32 GetPlayerSteamLevel() { return SteamUser()->GetPlayerSteamLevel(); }

	/**
	 * Gets the Steam ID of the account currently logged into the Steam client. This is commonly called the 'current user', or 'local user'.
	 * A Steam ID is a unique identifier for a Steam accounts, Steam groups, Lobbies and Chat rooms, and used to differentiate users in all parts of the Steamworks API.
	 *
	 * @return FSteamID
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	FSteamID GetSteamID() { return SteamUser()->GetSteamID().ConvertToUint64(); }

	/**
	 * Read captured audio data from the microphone buffer.
	 * The compressed data can be transmitted by your application and decoded back into raw audio data using DecompressVoice on the other side.
	 * The compressed data provided is in an arbitrary format and is not meant to be played directly.
	 * This should be called once per frame, and at worst no more than four times a second to keep the microphone input delay as low as possible.
	 * Calling this any less may result in gaps in the returned stream. It is recommended that you pass in an 8 kilobytes or larger destination buffer for compressed audio. Static buffers are -
	 * recommended for performance reasons. However, if you would like to allocate precisely the right amount of space for a buffer before each call you may use GetAvailableVoice to find out -
	 * how much data is available to be read.
	 *
	 * @param TArray<uint8> & VoiceData
	 * @return ESteamVoiceResult
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	ESteamVoiceResult GetVoice(TArray<uint8>& VoiceData);

	/**
	 * Gets the native sample rate of the Steam voice decoder.
	 * Using this sample rate for DecompressVoice will perform the least CPU processing.
	 * However, the final audio quality will depend on how well the audio device (and/or your application's audio output SDK) deals with lower sample rates.
	 * You may find that you get the best audio output quality when you ignore this function and use the native sample rate of your audio output device, which is usually 48000 or 44100.
	 *
	 * @return int32
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	int32 GetVoiceOptimalSampleRate() { return (uint32)SteamUser()->GetVoiceOptimalSampleRate(); }

	/**
	 * This starts the state machine for authenticating the game client with the game server.
	 * It is the client portion of a three-way handshake between the client, the game server, and the steam servers.
	 *
	 * @param TArray<uint8> & pAuthBlob
	 * @param FSteamID steamIDGameServer
	 * @param int32 unIPServer
	 * @param int32 usPortServer
	 * @param bool bSecure
	 * @return int32
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	int32 InitiateGameConnection(TArray<uint8>& pAuthBlob, FSteamID steamIDGameServer, int32 unIPServer, int32 usPortServer, bool bSecure);

	// TODO: RequestEncryptedAppTicket, RequestStoreAuthURL

	/**
	 * Starts voice recording.
	 * Once started, use GetAvailableVoice and GetVoice to get the data, and then call StopVoiceRecording when the user has released their push-to-talk hotkey or the game session has completed.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void StartVoiceRecording() { SteamUser()->StartVoiceRecording(); }

	/**
	 * Stops voice recording.
	 * Because people often release push-to-talk keys early, the system will keep recording for a little bit after this function is called. As such, GetVoice should continue to be called until -
	 * it returns k_EVoiceResultNotRecording, only then will voice recording be stopped.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void StopVoiceRecording() { SteamUser()->StopVoiceRecording(); }

	/**
	 * Notify the game server that we are disconnecting.
	 * This needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection call.
	 *
	 * @param int32 IPServer
	 * @param int32 PortServer
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|User")
	void TerminateGameConnection(int32 IPServer, int32 PortServer) { SteamUser()->TerminateGameConnection(IPServer, PortServer); }

	/**
	 * Checks if the user owns a specific piece of Downloadable Content (DLC).
	 * This can only be called after sending the users auth ticket to ISteamGameServer::BeginAuthSession
	 *
	 * @param FSteamID steamID
	 * @param int32 appID
	 * @return ESteamUserHasLicenseForAppResult
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|User")
	ESteamUserHasLicenseForAppResult UserHasLicenseForApp(FSteamID steamID, int32 appID) { return (ESteamUserHasLicenseForAppResult)SteamUser()->UserHasLicenseForApp(steamID.Value, appID); }

	/** Delegates */

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnClientGameServerDeny"))
	FOnClientGameServerDenyDelegate m_OnClientGameServerDeny;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnDurationControl"))
	FOnDurationControlDelegate m_OnDurationControl;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnEncryptedAppTicketResponse"))
	FOnEncryptedAppTicketResponseDelegate m_OnEncryptedAppTicketResponse;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnGameWebCallback"))
	FOnGameWebCallbackDelegate m_OnGameWeb;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnGetAuthSessionTicketResponse"))
	FOnGetAuthSessionTicketResponseDelegate m_OnGetAuthSessionTicketResponse;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnIPCFailure"))
	FOnIPCFailureDelegate m_IPCFailure;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnLicensesUpdated"))
	FOnLicensesUpdatedDelegate m_OnLicensesUpdated;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnMicroTxnAuthorizationResponse"))
	FOnMicroTxnAuthorizationResponseDelegate m_OnMicroTxnAuthorizationResponse;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnSteamServerConnectFailure"))
	FOnSteamServerConnectFailureDelegate m_OnSteamServerConnectFailure;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnSteamServersConnected"))
	FOnSteamServersConnectedDelegate m_OnSteamServersConnected;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnSteamServersDisconnected"))
	FOnSteamServersDisconnectedDelegate m_OnSteamServersDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnStoreAuthURLResponse"))
	FOnStoreAuthURLResponseDelegate m_OnStoreAuthURLResponse;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|User", meta = (DisplayName = "OnValidateAuthTicketResponse"))
	FOnValidateAuthTicketResponseDelegate m_OnValidateAuthTicketResponse;

protected:
private:
	int32 m_buffer = 8192;

	STEAM_CALLBACK_MANUAL(USteamUser, OnClientGameServerDeny, ClientGameServerDeny_t, OnClientGameServerDenyCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnDurationControl, DurationControl_t, OnDurationControlCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnEncryptedAppTicketResponse, EncryptedAppTicketResponse_t, OnEncryptedAppTicketResponseCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnGameWeb, GameWebCallback_t, OnGameWebCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnGetAuthSessionTicketResponse, GetAuthSessionTicketResponse_t, OnGetAuthSessionTicketResponseCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnIPCFailure, IPCFailure_t, OnIPCFailureCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnLicensesUpdated, LicensesUpdated_t, OnLicensesUpdatedCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnMicroTxnAuthorizationResponse, MicroTxnAuthorizationResponse_t, OnMicroTxnAuthorizationResponseCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnSteamServerConnectFailure, SteamServerConnectFailure_t, OnSteamServerConnectFailureCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnSteamServersConnected, SteamServersConnected_t, OnSteamServersConnectedCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnSteamServersDisconnected, SteamServersDisconnected_t, OnSteamServersDisconnectedCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnStoreAuthURLResponse, StoreAuthURLResponse_t, OnStoreAuthURLResponseCallback);
	STEAM_CALLBACK_MANUAL(USteamUser, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t, OnValidateAuthTicketResponseCallback);
};
