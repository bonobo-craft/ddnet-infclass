/* (c) Shereef Marzouk. See "licence Mod.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_MOD_H
#define GAME_SERVER_GAMEMODES_MOD_H
#include <game/server/gamecontroller.h>
#include <game/server/teams.h>
#include <game/server/entities/door.h>

#include <vector>
#include <map>

struct CScoreInitResult;
class CGameControllerMOD: public IGameController
//class CGameControllerMod: public CGameControllerDDRace
// class CGameControllerDDRace: public IGameController
{
public:

	CGameControllerMOD(class CGameContext *pGameServer);
	~CGameControllerMOD();

	CGameTeams m_Teams;

	std::map<int, std::vector<vec2> > m_TeleOuts;
	std::map<int, std::vector<vec2> > m_TeleCheckOuts;

	void InitTeleporter();
	virtual void Tick();

	std::shared_ptr<CScoreInitResult> m_pInitResult;
};
#endif // GAME_SERVER_GAMEMODES_MOD_H
