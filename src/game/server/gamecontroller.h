/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/vmath.h>
#include <engine/map.h>
// INFCROYA BEGIN ------------------------------------------------------------
#include <array>
#include <engine/shared/config.h>
// INFCROYA END ------------------------------------------------------------//

class CDoor;
#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#else
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif

/*
	Class: Game Controller
		Controls the main game logic. Keeping track of team and player score,
		winning conditions and specific game logic.
*/
class IGameController
{
	friend class CSaveTeam; // need access to GameServer() and Server()

	vec2 m_aaSpawnPoints[3][64];
	int m_aNumSpawnPoints[3];

	class CGameContext *m_pGameServer;
	class IServer *m_pServer;

protected:
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }

	struct CSpawnEval
	{
		CSpawnEval()
		{
			m_Got = false;
			m_FriendlyTeam = -1;
			m_Pos = vec2(100, 100);
		}

		vec2 m_Pos;
		bool m_Got;
		int m_FriendlyTeam;
		float m_Score;
	};

	float EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos);
	void EvaluateSpawnType(CSpawnEval *pEval, int Type);

	void ResetGame();

	char m_aMapWish[MAX_MAP_LENGTH];

	int m_RoundStartTick;
	int m_GameOverTick;
	int m_SuddenDeath;

	int m_MatchCount;
	int m_RoundCount;

	int m_GameFlags;
	int m_UnbalancedTick;
	bool m_ForceBalanced;

public:
	int m_Warmup;
	int m_GameStartTick;
	const char *m_pGameType;

	IGameController(class CGameContext *pGameServer);
	virtual ~IGameController();

	void DoWarmup(int Seconds);

	int m_TimeLimit;
	int m_RoundNum;

	void StartRound();
	void EndRound();
	void EndMatch();
	void ChangeMap(const char *pToMap);

		// info
	struct CGameInfo
	{
		int m_MatchCurrent;
		int m_MatchNum;
		int m_ScoreLimit;
		int m_TimeLimit;
	} m_GameInfo;


	void UpdateGameInfo(int ClientID);

	bool IsFriendlyFire(int ClientID1, int ClientID2);

	bool IsForceBalanced();

	/*

	*/
	virtual bool CanBeMovedOnBalance(int ClientID);

	virtual void Tick();

	virtual void Snap(int SnappingClient);

	/*
		Function: on_entity
			Called when the map is loaded to process an entity
			in the map.

		Arguments:
			index - Entity index.
			pos - Where the entity is located in the world.

		Returns:
			bool?
	*/
	//virtual bool OnEntity(int Index, vec2 Pos);
	virtual bool OnEntity(int Index, vec2 Pos, int Layer, int Flags, int Number = 0);

	/*
		Function: on_CCharacter_spawn
			Called when a CCharacter spawns into the game world.

		Arguments:
			chr - The CCharacter that was spawned.
	*/
	virtual void OnCharacterSpawn(class CCharacter *pChr);

	/*
		Function: on_CCharacter_death
			Called when a CCharacter in the world dies.

		Arguments:
			victim - The CCharacter that died.
			killer - The player that killed it.
			weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
	*/
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);

	//
	virtual bool CanSpawn(int Team, vec2 *pPos);

	/*

	*/
	virtual const char *GetTeamName(int Team);
	virtual int GetAutoTeam(int NotThisID);
	virtual bool CanJoinTeam(int Team, int NotThisID);
	int ClampTeam(int Team);

	virtual void PostReset();

	// DDRace

	float m_CurrentRecord;

	// INFCROYA BEGIN ------------------------------------------------------------
	bool IsSpawnable(vec2 Pos);
	bool IsWarmup() const;
	bool IsGameEnd() const;
	std::array<class CroyaPlayer*, 64> m_pCroyaPlayers{};
	class CGameControllerMOD* m_MOD;
	// INFCROYA END ------------------------------------------------------------//
};

#endif
