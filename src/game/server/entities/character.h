/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <engine/antibot.h>
#include <game/generated/protocol.h>
#include <game/generated/server_data.h>
#include <game/server/entity.h>
#include <game/server/save.h>
#include <infcroya/croyaplayer.h>

#include <game/gamecore.h>

class CAntibot;
class CGameTeams;
struct CAntibotCharacterData;

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

enum
{
	FAKETUNE_FREEZE = 1,
	FAKETUNE_SOLO = 2,
	FAKETUNE_NOJUMP = 4,
	FAKETUNE_NOCOLL = 8,
	FAKETUNE_NOHOOK = 16,
	FAKETUNE_JETPACK = 32,
	FAKETUNE_NOHAMMER = 64,
};

// INFCROYA BEGIN ------------------------------------------------------------
enum
{
	FREEZEREASON_FLASH = 0,
	FREEZEREASON_UNDEAD = 1
};
// INFCROYA END ------------------------------------------------------------//

class CCharacter : public CEntity
{
	MACRO_ALLOC_POOL_ID()

	friend class CSaveTee; // need to use core

public:
	struct WeaponStat
	{
		int m_AmmoRegenStart;
		int m_Ammo;
		int m_Ammocost;
		bool m_Got;

	} m_aWeapons[NUM_WEAPONS];
	//character's size
	static const int ms_PhysSize = 28;

	CCharacter(CGameWorld *pWorld);

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	virtual int NetworkClipped(int SnappingClient);
	virtual int NetworkClipped(int SnappingClient, vec2 CheckPos);

	bool IsGrounded();

	void SetWeapon(int W);
	void SetSolo(bool Solo);
	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	void HandleNinja();
	void HandleJetpack();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	void FireWeapon();

	void Die(int Killer, int Weapon);
	bool TakeDamageDDNet(vec2 Force, int Dmg, int From, int Weapon);
	bool TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon);

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);

	void GiveWeapon(int Weapon, int Ammo, bool Remove = false);
	void GiveNinja();
	void GiveNinjaIfGrounded();
	void RemoveNinja();

	void SetEmote(int Emote, int Tick);

	void Rescue();

	int NeededFaketuning() { return m_NeededFaketuning; }
	bool IsAlive() const { return m_Alive; }
	bool IsPaused() const { return m_Paused; }
	class CPlayer *GetPlayer() { return m_pPlayer; }

	// INFCROYA BEGIN ------------------------------------------------------------
	void DestroyMyBot();
	void DestroyAllBots();
	void DestroyBotByID(int BotID);
	void SpawnBot();
	void RegainBotControl();
	void SwitchTaxi();
	void ResetTaxi();
	void ExitTaxiAsPassenger();
	void ExitTaxiAsDriver();
	bool IsTaxiDriver();
	bool IsTaxiPassenger();
	void SetNormalEmote(int Emote);

	bool IsHuman() const;
	bool IsZombie() const;

	void SetInfected(bool Infected);

	void SetCroyaPlayer(class CroyaPlayer* CroyaPlayer);
	class CroyaPlayer* GetCroyaPlayer();

	void ResetWeaponsHealth();

	int GetActiveWeapon() const;
	void SetReloadTimer(int ReloadTimer);
	void SetNumObjectsHit(int NumObjectsHit);
	void Infect(int From);
	bool IncreaseOverallHp(int Amount);
	int GetArmor() const;
	int GetHealthArmorSum() const;
	void SetHealthArmor(int Health, int Armor);
	int VacantBotId();

	CCharacterCore& GetCharacterCore();
	bool m_FirstShot;
	vec2 m_FirstShotCoord;
	int m_BarrierHintID;
	array<int> m_BarrierHintIDs;
	int m_RespawnPointID;
	int m_BotClientID;

	void Stun(float Time);
	void Unstun();
	bool Stunned();
	void Poison(int Count, int From);
	void SlowMotionEffect(float duration);
	bool IsInSlowMotion() const;

	void DestroyChildEntities();

	bool IsHookProtected() const;
	void SetHookProtected(bool HookProtected);

	CNetObj_PlayerInput& GetInput();

	bool FindPortalPosition(vec2 Pos, vec2& Res);

	void SaturateVelocity(vec2 Force, float MaxSpeed);
	bool m_IsInSlowMotion;
	int m_SlowMotionTick;

	bool IsBot();
	int m_HookDmgTick;
	int m_InAirTick;
	int GetInfWeaponID(int WID);

	int GetLastNoAmmoSound() const;
	void SetLastNoAmmoSound(int LastNoAmmoSound);

	int GetLastNoAttachSound() const;
	void SetLastNoAttachSound(int LastNoAttachSound);

	bool m_IsBot;
	// INFCROYA END ------------------------------------------------------------//

private:
	// player controlling this character
	class CPlayer *m_pPlayer;

	bool m_Alive;
	bool m_Paused;
	int m_NeededFaketuning;

	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	// INFCROYA BEGIN ------------------------------------------------------------
	bool m_Infected;
	CroyaPlayer* m_pCroyaPlayer;
	int m_HeartID;
	int m_TaxiID;
	int m_HookProtID;
	int m_NormalEmote;
	bool m_IsFrozen;
	bool m_IsStunned;
	int m_StunTime;
	int m_FreezeReason;
	int m_LastFreezer;

	int m_Poison;
	int m_PoisonTick;
	int m_PoisonFrom;
	bool m_HookProtected;
	// INFCROYA END ------------------------------------------------------------//


	int m_LastWeapon;
	int m_QueuedWeapon;

	int m_ReloadTimer;
	int m_AttackTick;

	int m_DamageTaken;

	int m_EmoteType;
	int m_EmoteStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;
	int m_LastNoAttachSound;

	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevPrevInput;
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;
	CNetObj_PlayerInput m_SavedInput;
	int m_NumInputs;
	int m_Jumped;

	int m_DamageTakenTick;

	int m_Health;
	int m_Armor;

	// ninja
	struct
	{
		vec2 m_ActivationDir;
		int m_ActivationTick;
		int m_CurrentMoveTime;
		int m_OldVelAmount;
	} m_Ninja;

	// the player core for the physics
	CCharacterCore m_Core;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

	// DDRace

	void SnapCharacter(int SnappingClient, int ID);
	static bool IsSwitchActiveCb(int Number, void *pUser);
	void HandleTiles(int Index);
	float m_Time;
	int m_LastBroadcast;
	void DDRaceInit();
	void HandleSkippableTiles(int Index);
	void SetRescue();
	void DDRaceTick();
	void DDRacePostCoreTick();
	void HandleBroadcast();
	void HandleTuneLayer();
	void SendZoneMsgs();
	IAntibot *Antibot();

	bool m_SetSavePos;
	CSaveTee m_RescueTee;
	bool m_Solo;

public:
	CGameTeams *Teams();
	void FillAntibot(CAntibotCharacterData *pData);
	void Pause(bool Pause);
	bool Freeze(int Time);
	bool Freeze();
	bool UnFreeze();
	bool Frozen();

	void GiveAllWeapons();
	int m_DDRaceState;
	int Team();
	bool CanCollide(int ClientID);
	bool CanCollideInf(int ClientID); // INFLUCK
	bool SameTeam(int ClientID);
	bool m_Super;
	bool m_SuperJump;
	bool m_Jetpack;
	bool m_NinjaJetpack;
	int m_TeamBeforeSuper;
	int m_FreezeTime;
	int m_FreezeTick;
	bool m_FrozenLastTick;
	bool m_DeepFreeze;
	bool m_EndlessHook;
	bool m_FreezeHammer;
	enum
	{
		HIT_ALL = 0,
		DISABLE_HIT_HAMMER = 1,
		DISABLE_HIT_SHOTGUN = 2,
		DISABLE_HIT_GRENADE = 4,
		DISABLE_HIT_LASER = 8
	};
	int m_Hit;
	int m_TuneZone;
	int m_TuneZoneOld;
	int m_PainSoundTimer;
	int m_LastMove;
	int m_StartTime;
	vec2 m_PrevPos;
	int m_TeleCheckpoint;
	int m_CpTick;
	int m_CpActive;
	int m_CpLastBroadcast;
	float m_CpCurrent[25];
	int m_TileIndex;
	int m_TileFIndex;

	int m_MoveRestrictions;

	vec2 m_Intersection;
	int64 m_LastStartWarning;
	int64 m_LastRescue;
	bool m_LastRefillJumps;
	bool m_LastPenalty;
	bool m_LastBonus;
	vec2 m_TeleGunPos;
	bool m_TeleGunTeleport;
	bool m_IsBlueTeleGunTeleport;
	int m_StrongWeakID;

	int m_SpawnTick;
	int m_WeaponChangeTick;

	// Setters/Getters because i don't want to modify vanilla vars access modifiers
	int GetLastWeapon() { return m_LastWeapon; };
	void SetLastWeapon(int LastWeap) { m_LastWeapon = LastWeap; };
	int GetActiveWeapon() { return m_Core.m_ActiveWeapon; };
	void SetActiveWeapon(int ActiveWeap) { m_Core.m_ActiveWeapon = ActiveWeap; };
	void SetLastAction(int LastAction) { m_LastAction = LastAction; };
	int GetArmor() { return m_Armor; };
	void SetArmor(int Armor) { m_Armor = Armor; };
	CCharacterCore *GetpCore() { return &m_Core; };
	CCharacterCore GetCore() { return m_Core; };
	void SetCore(CCharacterCore Core) { m_Core = Core; };
	CCharacterCore *Core() { return &m_Core; };
	bool GetWeaponGot(int Type) { return m_aWeapons[Type].m_Got; };
	void SetWeaponGot(int Type, bool Value) { m_aWeapons[Type].m_Got = Value; };
	int GetWeaponAmmo(int Type) { return m_aWeapons[Type].m_Ammo; };
	void SetWeaponAmmo(int Type, int Value) { m_aWeapons[Type].m_Ammo = Value; };
	bool IsAlive() { return m_Alive; };
	void SetEmoteType(int EmoteType) { m_EmoteType = EmoteType; };
	void SetEmoteStop(int EmoteStop) { m_EmoteStop = EmoteStop; };
	void SetNinjaActivationDir(vec2 ActivationDir) { m_Ninja.m_ActivationDir = ActivationDir; };
	void SetNinjaActivationTick(int ActivationTick) { m_Ninja.m_ActivationTick = ActivationTick; };
	void SetNinjaCurrentMoveTime(int CurrentMoveTime) { m_Ninja.m_CurrentMoveTime = CurrentMoveTime; };

	bool HasTelegunGun() { return m_Core.m_HasTelegunGun; };
	bool HasTelegunGrenade() { return m_Core.m_HasTelegunGrenade; };
	bool HasTelegunLaser() { return m_Core.m_HasTelegunLaser; };
	bool m_FreeTaxi;
	bool m_TaxiPassenger;
};

enum
{
	DDRACE_NONE = 0,
	DDRACE_STARTED,
	DDRACE_CHEAT, // no time and won't start again unless ordered by a mod or death
	DDRACE_FINISHED
};

#endif
