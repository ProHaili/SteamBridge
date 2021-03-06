// Copyright 2020 Russ 'trdwll' Treadwell <trdwll.com>. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steam.h"
#include "SteamEnums.h"
#include "SteamStructs.h"
#include "UObject/NoExportTypes.h"

#include "SteamGameServer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssociateWithClanResultDelegate, ESteamResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnComputeNewPlayerCompatibilityResultDelegate, ESteamResult, Result, int32, PlayersThatDontLikeCandidate, int32, PlayersThatDoesntLikeCandidate,
	int32, ClanPlayersThatDontLikeCandidate, FSteamID, SteamIDCandidate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGSClientApproveDelegate, FSteamID, SteamID, FSteamID, OwnerSteamID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGSClientDenyDelegate, FSteamID, SteamID, ESteamDenyReason, DenyReason, FString, OptionalText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGSClientGroupStatusDelegate, FSteamID, SteamIDUser, FSteamID, SteamIDGroup, bool, bMember, bool, bOfficer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGSClientKickDelegate, FSteamID, SteamID, ESteamDenyReason, DenyReason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSPolicyResponseDelegate, bool, bSecure);

/**
 * Provides the core of the Steam Game Servers API.
 * https://partner.steamgames.com/doc/api/ISteamGameServer
 */
UCLASS()
class STEAMBRIDGE_API USteamGameServer final : public UObject
{
	GENERATED_BODY()

public:
	USteamGameServer();
	~USteamGameServer();

	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "Steam Game Server", CompactNodeTitle = "SteamGameServer"))
	static USteamGameServer* GetSteamGameServer() { return USteamGameServer::StaticClass()->GetDefaultObject<USteamGameServer>(); }

	/**
	 * Associate this game server with this clan for the purposes of computing player compatibility.
	 *
	 * @param FSteamID SteamIDClan
	 * @return FSteamAPICall
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	FSteamAPICall AssociateWithClan(FSteamID SteamIDClan) const { return SteamGameServer()->AssociateWithClan(SteamIDClan.Value); }

	/**
	 * Authenticate the ticket from the entity Steam ID to be sure it is valid and isn't reused.
	 * The ticket is created on the entity with ISteamUser::GetAuthSessionTicket or GetAuthSessionTicket and then needs to be provided over the network for the other end to validate.
	 * This registers for ValidateAuthTicketResponse_t callbacks if the entity goes offline or cancels the ticket. See EAuthSessionResponse for more information.
	 * When the multiplayer session terminates you must call EndAuthSession.
	 *
	 * @param TArray<uint8> AuthTicket
	 * @param FSteamID SteamID
	 * @return ESteamBeginAuthSessionResult
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	ESteamBeginAuthSessionResult BeginAuthSession(TArray<uint8> AuthTicket, FSteamID SteamID);

	/**
	 * Checks if the game server is logged on.
	 *
	 * @return bool
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	bool BLoggedOn() const { return SteamGameServer()->BLoggedOn(); }

	/**
	 * Checks whether the game server is in "Secure" mode.
	 *
	 * @return bool
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	bool BSecure() const { return SteamGameServer()->BSecure(); }

	/**
	 * Cancels an auth ticket received from ISteamUser::GetAuthSessionTicket. This should be called when no longer playing with the specified entity.
	 *
	 * @param FHAuthTicket AuthTicketHandle
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void CancelAuthTicket(FHAuthTicket AuthTicketHandle) { SteamGameServer()->CancelAuthTicket(AuthTicketHandle.Value); }

	/**
	 * Clears the whole list of key/values that are sent in rules queries.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void ClearAllKeyValues() { SteamGameServer()->ClearAllKeyValues(); }

	// TODO: ComputeNewPlayerCompatibility

	/**
	 * Tells the Steam master servers whether or not you want to be active.
	 * If this is enabled then the server will talk to the master servers, if it's not then incoming messages are ignored and heartbeats will not be sent.
	 *
	 * @param bool bActive
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void EnableHeartbeats(bool bActive) { SteamGameServer()->EnableHeartbeats(bActive); }

	/**
	 * Ends an auth session that was started with BeginAuthSession. This should be called when no longer playing with the specified entity.
	 *
	 * @param FSteamID SteamID
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void EndAuthSession(FSteamID SteamID) { SteamGameServer()->EndAuthSession(SteamID.Value); }

	/**
	 * Force a heartbeat to the Steam master servers at the next opportunity.
	 * You usually don't need to use this.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void ForceHeartbeat() { SteamGameServer()->ForceHeartbeat(); }

	/**
	 * Retrieve a authentication ticket to be sent to the entity who wishes to authenticate you.
	 * After calling this you can send the ticket to the entity where they can then call ISteamUser::BeginAuthSession to verify this entities integrity.
	 * When creating a ticket for use by the ISteamUserAuth/AuthenticateUserTicket Web API, the calling application should wait for the GetAuthSessionTicketResponse_t callback generated by the API -
	 * call before attempting to use the ticket to ensure that the ticket has been communicated to the server. If this callback does not come in a timely fashion (10 - 20 seconds), then your -
	 * client is not connected to Steam, and the AuthenticateUserTicket call will fail because it can not authenticate the user.
	 *
	 * @param TArray<uint8> & AuthTicket
	 * @return FHAuthTicket
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	FHAuthTicket GetAuthSessionTicket(TArray<uint8>& AuthTicket);

	// TODO: GetNextOutgoingPacket
	// TODO: GetPublicIP

	/**
	 * Gets the Steam ID of the game server.
	 *
	 * @return FSteamID
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	FSteamID GetSteamID() const { return SteamGameServer()->GetSteamID().ConvertToUint64(); }

	// TODO: HandleIncomingPacket
	// TODO: InitGameServer

	/**
	 * Begin process of logging the game server out of steam.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void LogOff() { SteamGameServer()->LogOff(); }

	/**
	 * Begin process to login to a persistent game server account.
	 *
	 * @param const FString & Token
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void LogOn(const FString& Token) { SteamGameServer()->LogOn(TCHAR_TO_UTF8(*Token)); }

	/**
	 * Login to a generic, anonymous account.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void LogOnAnonymous() { SteamGameServer()->LogOnAnonymous(); }

	/**
	 * Checks if a user is in the specified Steam group.
	 *
	 * @param FSteamID SteamIDUser
	 * @param FSteamID SteamIDGroup
	 * @return bool
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	bool RequestUserGroupStatus(FSteamID SteamIDUser, FSteamID SteamIDGroup) const { return SteamGameServer()->RequestUserGroupStatus(SteamIDUser.Value, SteamIDGroup.Value); }

	/**
	 * Sets the number of bot/AI players on the game server. The default value is 0.
	 *
	 * @param int32 BotPlayers
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetBotPlayerCount(int32 BotPlayers) { SteamGameServer()->SetBotPlayerCount(BotPlayers); }

	/**
	 * Sets the whether this is a dedicated server or a listen server. The default is listen server.
	 * NOTE: This only be set before calling LogOn or LogOnAnonymous.
	 *
	 * @param bool bDedicated
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetDedicatedServer(bool bDedicated) { SteamGameServer()->SetDedicatedServer(bDedicated); }

	/**
	 * Sets a string defining the "gamedata" for this server, this is optional, but if set it allows users to filter in the matchmaking/server-browser interfaces based on the value.
	 * This is usually formatted as a comma or semicolon separated list.
	 * Don't set this unless it actually changes, its only uploaded to the master once; when acknowledged.
	 *
	 * @param const FString & GameData
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetGameData(const FString& GameData) { SteamGameServer()->SetGameData(TCHAR_TO_UTF8(*GameData)); }

	/**
	 * Sets the game description. Setting this to the full name of your game is recommended.
	 * NOTE: This is required for all game servers and can only be set before calling LogOn or LogOnAnonymous.
	 *
	 * @param const FString GameDescription
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetGameDescription(const FString GameDescription) { SteamGameServer()->SetGameDescription(TCHAR_TO_UTF8(*GameDescription)); }

	/**
	 * Sets a string defining the "gametags" for this server, this is optional, but if set it allows users to filter in the matchmaking/server-browser interfaces based on the value.
	 * This is usually formatted as a comma or semicolon separated list.
	 * Don't set this unless it actually changes, its only uploaded to the master once; when acknowledged.
	 *
	 * @param const FString & GameTags
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetGameTags(const FString& GameTags) { SteamGameServer()->SetGameTags(TCHAR_TO_UTF8(*GameTags)); }

	/**
	 * Changes how often heartbeats are sent to the Steam master servers.
	 * You usually don't need to use this.
	 *
	 * @param int32 HeartbeatInterval
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetHeartbeatInterval(int32 HeartbeatInterval) { SteamGameServer()->SetHeartbeatInterval(HeartbeatInterval); }

	/**
	 * Add/update a rules key/value pair.
	 *
	 * @param const FString & Key
	 * @param const FString & Value
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetKeyValue(const FString& Key, const FString& Value) { SteamGameServer()->SetKeyValue(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value)); }

	/**
	 * Sets the name of map to report in the server browser.
	 *
	 * @param const FString & MapName
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetMapName(const FString& MapName) { SteamGameServer()->SetMapName(TCHAR_TO_UTF8(*MapName)); }

	/**
	 * Sets the maximum number of players allowed on the server at once.
	 * This value may be changed at any time.
	 *
	 * @param int32 PlayersMax
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetMaxPlayerCount(int32 PlayersMax) { SteamGameServer()->SetMaxPlayerCount(PlayersMax); }

	/**
	 * Sets the game directory.
	 * This should be the same directory game where gets installed into. Just the folder name, not the whole path. I.e. "Spacewar".
	 * NOTE: This is required for all game servers and can only be set before calling LogOn or LogOnAnonymous.
	 *
	 * @param const FString & ModDir
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetModDir(const FString& ModDir) { SteamGameServer()->SetModDir(TCHAR_TO_UTF8(*ModDir)); }

	/**
	 * Set whether the game server will require a password once when the user tries to join.
	 *
	 * @param bool bPasswordProtected
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetPasswordProtected(bool bPasswordProtected) { SteamGameServer()->SetPasswordProtected(bPasswordProtected); }

	/**
	 * Sets the game product identifier. This is currently used by the master server for version checking purposes.
	 * Converting the games app ID to a string for this is recommended.
	 * NOTE: This is required for all game servers and can only be set before calling LogOn or LogOnAnonymous.
	 *
	 * @param const FString & Product
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetProduct(const FString& Product) { SteamGameServer()->SetProduct(TCHAR_TO_UTF8(*Product)); }

	/**
	 * Region identifier. This is an optional field, the default value is an empty string, meaning the "world" region.
	 *
	 * @param const FString & Region
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetRegion(const FString& Region) { SteamGameServer()->SetRegion(TCHAR_TO_UTF8(*Region)); }

	/**
	 * Sets the name of server as it will appear in the server browser.
	 *
	 * @param const FString & ServerName
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetServerName(const FString& ServerName) { SteamGameServer()->SetServerName(TCHAR_TO_UTF8(*ServerName)); }

	/**
	 * Set whether the game server allows spectators, and what port they should connect on. The default value is 0, meaning the service is not used.
	 *
	 * @param int32 SpectatorPort
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetSpectatorPort(int32 SpectatorPort) { SteamGameServer()->SetSpectatorPort(FMath::Clamp<uint16>(SpectatorPort, 0, 65535)); }

	/**
	 * Sets the name of the spectator server. This is only used if spectator port is nonzero.
	 *
	 * @param const FString & SpectatorServerName
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	void SetSpectatorServerName(const FString& SpectatorServerName) { SteamGameServer()->SetSpectatorServerName(TCHAR_TO_UTF8(*SpectatorServerName)); }

	/**
	 * Checks if the user owns a specific piece of Downloadable Content (DLC).
	 * This can only be called after sending the users auth ticket to BeginAuthSession/
	 *
	 * @param FSteamID SteamID
	 * @param int32 AppID
	 * @return ESteamUserHasLicenseForAppResult
	 */
	UFUNCTION(BlueprintCallable, Category = "SteamBridgeCore|GameServer")
	ESteamUserHasLicenseForAppResult UserHasLicenseForApp(FSteamID SteamID, int32 AppID) { return (ESteamUserHasLicenseForAppResult)SteamGameServer()->UserHasLicenseForApp(SteamID.Value, AppID); }

	/**
	 * Checks if the master server has alerted us that we are out of date.
	 * This reverts back to false after calling this function.
	 *
	 * @return bool
	 */
	UFUNCTION(BlueprintPure, Category = "SteamBridgeCore|GameServer")
	bool WasRestartRequested() const { return SteamGameServer()->WasRestartRequested(); }

	/** Delegates */
	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnAssociateWithClanResult"))
	FOnAssociateWithClanResultDelegate m_OnAssociateWithClanResult;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnComputeNewPlayerCompatibilityResult"))
	FOnComputeNewPlayerCompatibilityResultDelegate m_OnComputeNewPlayerCompatibilityResult;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnGSClientApprove"))
	FOnGSClientApproveDelegate m_OnGSClientApprove;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnGSClientDeny"))
	FOnGSClientDenyDelegate m_OnGSClientDeny;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnGSClientGroupStatus"))
	FOnGSClientGroupStatusDelegate m_OnGSClientGroupStatus;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnGSClientKick"))
	FOnGSClientKickDelegate m_OnGSClientKick;

	UPROPERTY(BlueprintAssignable, Category = "SteamBridgeCore|GameServer", meta = (DisplayName = "OnGSPolicyResponse"))
	FOnGSPolicyResponseDelegate m_OnGSPolicyResponse;

protected:
private:
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnAssociateWithClanResult, AssociateWithClanResult_t, OnAssociateWithClanResultCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnComputeNewPlayerCompatibilityResult, ComputeNewPlayerCompatibilityResult_t, OnComputeNewPlayerCompatibilityResultCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnGSClientApprove, GSClientApprove_t, OnGSClientApproveCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnGSClientDeny, GSClientDeny_t, OnGSClientDenyCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnGSClientGroupStatus, GSClientGroupStatus_t, OnGSClientGroupStatusCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnGSClientKick, GSClientKick_t, OnGSClientKickCallback);
	STEAM_GAMESERVER_CALLBACK_MANUAL(USteamGameServer, OnGSPolicyResponse, GSPolicyResponse_t, OnGSPolicyResponseCallback);
};
