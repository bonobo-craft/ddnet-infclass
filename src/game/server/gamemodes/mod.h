/* (c) Shereef Marzouk. See "licence Mod.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_MOD_H
#define GAME_SERVER_GAMEMODES_MOD_H
#include <unordered_map>
#include <game/server/gamecontroller.h>
#include <game/server/teams.h>
#include <game/server/entities/door.h>
#include <game/server/entities/flag.h>

#include <vector>
#include <map>

struct CScoreInitResult;
class CGameControllerMOD: public IGameController
{
public:

	CGameControllerMOD(class CGameContext *pGameServer);
	~CGameControllerMOD();

	void Snap(int SnappingClient);

	CGameTeams m_Teams;

	std::map<int, std::vector<vec2> > m_TeleOuts;
	std::map<int, std::vector<vec2> > m_TeleCheckOuts;

	void InitTeleporter();
	void Tick();

	bool IsCroyaWarmup(); // First 10 secs after game start
	bool RoundJustStarted();

	void OnRoundStart();
	void StartInitialInfection();
	void TurnDefaultIntoRandomHuman();
	void UnlockPositions();

	void OnRoundEnd();
	bool DoWincheckMatch();
	void DoWincheckRound();
	
	const vec2 *GetMotherPos();
	void OnCharacterSpawn(class CCharacter* pChr);
	void SpawnOnInfZone(CCharacter* pChr);
	int OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon) override;

	bool IsFriendlyFire(int ClientID1, int ClientID2) const;

	int GetIZombieCount() const;
	int GetZombieCount() const;
	int GetHumanCount() const;
	bool IsEveryoneInfected() const;

	void OnPlayerDisconnect(class CPlayer* pPlayer);
	void OnPlayerConnect(class CPlayer* pPlayer);

	std::array<CroyaPlayer*, 64> GetCroyaPlayers();
	void ResetFinalExplosion();

	bool IsExplosionStarted() const;
	int GetRealPlayerNum() const;
	int NUM_TEAMS=2;
	

	std::vector<class CCircle*>& GetCircles();
	std::vector<class CInfCircle*>& GetInfCircles();

	std::shared_ptr<CScoreInitResult> m_pInitResult;
	bool m_InfectedStarted;

	
private: 
	std::array<CroyaPlayer*, 64> players{};
	std::unordered_map<int, class IClass*> classes;
	class Geolocation* geolocation;
	std::vector<int> humans;
	//class LuaLoader* lua;
	//TBD
	std::vector<class CCircle*> circles;
	std::vector<class CInfCircle*> inf_circles;

	void ResetHumansToDefault() const;
	void FlagTick();

	bool m_ExplosionStarted;
	int m_MapWidth;
	int m_MapHeight;
	int* m_GrowingMap;

	// game
	CFlag *m_apFlag;
	long unsigned int m_apFlagSpotId;
	std::vector<vec2> flag_positions;
};
enum // infclass 0.6 copypaste
{
	INFWEAPON_NONE,
	INFWEAPON_HAMMER,
	INFWEAPON_GUN,
	INFWEAPON_SHOTGUN,
	INFWEAPON_GRENADE,
	INFWEAPON_RIFLE,
	INFWEAPON_NINJA,

	INFWEAPON_ENGINEER_RIFLE,

	INFWEAPON_SNIPER_RIFLE,

	INFWEAPON_SOLDIER_GRENADE,

	INFWEAPON_SCIENTIST_GRENADE,
	INFWEAPON_SCIENTIST_RIFLE,

	INFWEAPON_MEDIC_GRENADE,
	INFWEAPON_MEDIC_RIFLE,
	INFWEAPON_MEDIC_SHOTGUN,

	INFWEAPON_HERO_GRENADE,
	INFWEAPON_HERO_RIFLE,
	INFWEAPON_HERO_SHOTGUN,

	INFWEAPON_BIOLOGIST_SHOTGUN,
	INFWEAPON_BIOLOGIST_RIFLE,

	INFWEAPON_LOOPER_RIFLE,
	INFWEAPON_LOOPER_GRENADE,

	INFWEAPON_NINJA_HAMMER,
	INFWEAPON_NINJA_GRENADE,

	INFWEAPON_MERCENARY_GUN,
	INFWEAPON_MERCENARY_GRENADE,

	NB_INFWEAPON
};
#endif // GAME_SERVER_GAMEMODES_MOD_H