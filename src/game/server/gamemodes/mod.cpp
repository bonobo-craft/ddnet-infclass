/* (c) Shereef Marzouk. See "licence Mod.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our Mod needs. */
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "mod.h"
#include "gamemode.h"
#include <game/server/entities/flag.h>

#include <infcroya/croyaplayer.h>
#include <infcroya/classes/default.h>
#include <infcroya/classes/biologist.h>
#include <infcroya/classes/smoker.h>
#include <infcroya/classes/boomer.h>
#include <infcroya/classes/hunter.h>
#include <infcroya/classes/engineer.h>
#include <infcroya/classes/soldier.h>
#include <infcroya/classes/scientist.h>
#include <infcroya/classes/medic.h>
#include <infcroya/classes/mercenary.h>
#include <infcroya/classes/mother.h>
#include <infcroya/classes/poisoner.h>
#include <infcroya/classes/freezer.h>
#include <infcroya/classes/hero.h>
#include <infcroya/classes/sniper.h>
#include <infcroya/classes/bat.h>
#include <infcroya/classes/worker.h>
#include <infcroya/classes/parasite.h>
#include <infcroya/entities/inf-circle.h>
#include <infcroya/entities/circle.h>

CGameControllerMOD::CGameControllerMOD(class CGameContext *pGameServer) :
		IGameController(pGameServer), m_Teams(pGameServer), m_pInitResult(nullptr)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_NAME : GAME_NAME;

	InitTeleporter();

	//IGameController::m_MOD = this; // temporarily, todo: avoid this
	lua = nullptr;

	m_ExplosionStarted = false;
	m_InfectedStarted = false;

	classes[Class::DEFAULT] = new CDefault();
	classes[Class::BIOLOGIST] = new CBiologist();
	classes[Class::ENGINEER] = new CEngineer();
	classes[Class::MEDIC] = new CMedic();
	classes[Class::SOLDIER] = new CSoldier();
	classes[Class::SCIENTIST] = new CScientist();
	classes[Class::MERCENARY] = new CMercenary();
	classes[Class::HERO] = new CHero();
	classes[Class::SNIPER] = new CSniper();
	classes[Class::SMOKER] = new CSmoker();
	classes[Class::BOOMER] = new CBoomer();
	classes[Class::HUNTER] = new CHunter();
	classes[Class::MOTHER] = new CMother();
	classes[Class::POISONER] = new CPoisoner();
	classes[Class::FREEZER] = new CFreezer();
	classes[Class::BAT] = new CBat();
	classes[Class::WORKER] = new CWorker();
	classes[Class::PARASITE] = new CParasite();
  
	//g_Config.m_SvTimelimit = 4 // TODO: load from map config
}

CGameControllerMOD::~CGameControllerMOD()
{
	// Nothing to clean
}

void CGameControllerMOD::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	for (CroyaPlayer* each : players) {
		if (!each)
			continue;

		if (each->IsZombie() && each->IsRespawnPointPlaced() && each->GetRespawnPointsNum() >= 1 && each->GetCharacter()) {
			// Respawn point
			CNetObj_Pickup* pP = static_cast<CNetObj_Pickup*>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, each->GetCharacter()->m_RespawnPointID, sizeof(CNetObj_Pickup)));
			if (!pP)
				return;

			pP->m_X = (int)each->GetRespawnPointPos().x;
			pP->m_Y = (int)each->GetRespawnPointPos().y;
			//pP->m_Type = PICKUP_ARMOR;
			//TBD
		}
	}
}

void CGameControllerMOD::OnRoundStart()
{
	char aBuf[256];
	if (!lua) { // not sure yet if OnRoundStart is run only once
		std::string path_to_lua("maps/");
		path_to_lua += g_Config.m_SvMap;
		path_to_lua += ".lua";
		//lua = new LuaLoader(GameServer());
		//lua->Load(path_to_lua.c_str());
		//lua->Init(GetRealPlayerNum());

		//// positions.size() should be equal to radiuses.size()
		//// safezone circles
		//flag_positions = lua->GetFlagsPositions();

		//auto positions = lua->GetCirclePositions();
		//auto radiuses = lua->GetCircleRadiuses();
		//auto min_radiuses = lua->GetCircleMinRadiuses();
		//auto shrink_speeds = lua->GetCircleShrinkSpeeds();
		const int TILE_SIZE = 32;

		//for (size_t i = 0; i < positions.size(); i++) {
		//	int x = positions[i].x * TILE_SIZE;
		//	int y = positions[i].y * TILE_SIZE;
		//	circles.push_back(new CCircle(&GameServer()->m_World, vec2(x, y), -1, radiuses[i], min_radiuses[i], shrink_speeds[i]));
		//}

		// infection zone circles
		//auto inf_positions = lua->GetInfCirclePositions();
		//auto inf_radiuses = lua->GetInfCircleRadiuses();
		//m_GrowingMap[(int)inf_positions[0].y * m_MapWidth + (int)inf_positions[0].x] = 6; // final explosion start pos (?)
		//for (size_t i = 0; i < inf_positions.size(); i++) {
		//	const int TILE_SIZE = 32;
		//	int x = inf_positions[i].x * TILE_SIZE;
		//	int y = inf_positions[i].y * TILE_SIZE;
		//	inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(x, y), -1, inf_radiuses[i]));
		//}
		//////inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(30, 30), -1, 100));
	}
	m_InfectedStarted = true;
	TurnDefaultIntoRandomHuman();
	UnlockPositions();

	for (CCircle* circle : circles) {
		if (circle->GetRadius() > circle->GetMinRadius())
			circle->SetRadius(circle->GetRadius() - circle->GetShrinkSpeed());
	}

	if ( flag_positions.size() > 0) {
		m_apFlagSpotId = rand() % flag_positions.size();
		str_format(aBuf, sizeof(aBuf), "First flag spot id %ld of %ld", m_apFlagSpotId, flag_positions.size() - 1);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
		GameServer()->SendChatTarget(-1, aBuf);

		vec2 newPos = flag_positions[m_apFlagSpotId];
		str_format(aBuf, sizeof(aBuf), "Flag is spawned on %f, %f", newPos.x, newPos.y);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
		m_apFlag = new CFlag(&GameServer()->m_World, 0, newPos);

		str_format(aBuf, sizeof(aBuf), "Flag is spawned");
		GameServer()->SendChatTarget(-1, aBuf);

	}


}

void CGameControllerMOD::TurnDefaultIntoRandomHuman()
{
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (!each->GetCharacter())
			continue;
		if (each->GetClassNum() == Class::DEFAULT && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS) {
			each->TurnIntoRandomHuman();
			each->SetBeenOnRoundStart();
			each->GetCharacter()->IncreaseArmor(10);
		}
	}
}

void CGameControllerMOD::UnlockPositions()
{
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (!each->GetCharacter())
			continue;
		if (!each->GetClass())
			continue;
		each->GetClass()->UnlockPosition(each->GetCharacter());
	}
}

void CGameControllerMOD::Tick()
{
	IGameController::Tick();
	m_Teams.ProcessSaveTeam();

	if(m_pInitResult != nullptr && m_pInitResult.use_count() == 1)
	{
		if(m_pInitResult->m_Done)
		{
			m_CurrentRecord = m_pInitResult->m_CurrentRecord;
		}
		m_pInitResult = nullptr;
	}
}

void CGameControllerMOD::OnRoundEnd()
{
	m_InfectedStarted = false;
	ResetFinalExplosion();
	ResetHumansToDefault();
	delete lua;
	lua = nullptr;
	circles.clear();
	inf_circles.clear();
    if (m_apFlag != 0) {
		m_apFlag->Destroy();
		m_apFlag = 0;
	}
	IGameController::EndMatch(); // Endmatch instead of endround
}

bool CGameControllerMOD::DoWincheckMatch()
{
	return false;
}

void CGameControllerMOD::DoWincheckRound()
{
}

void CGameControllerMOD::OnCharacterSpawn(CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();

	players[ClientID]->SetCharacter(pChr);
	players[ClientID]->OnCharacterSpawn(pChr);

	if (pChr->IsZombie() && GetZombieCount() == 1) {
		pChr->IncreaseArmor(10); // +10 armor for lonely zombie
	}

	const vec2 *MotherPos = GetMotherPos();

	if (!pChr->IsZombie() || IsCroyaWarmup() || m_ExplosionStarted || !pChr->GetCroyaPlayer())
	{
		if (players[ClientID]->GetRespawnPointsNum() <= 0)
		{
			players[ClientID]->SetRespawnPointPlaced(false);
		}
	}

	if (pChr->GetCroyaPlayer()->GetClassNum() == Class::WORKER && GetMotherPos() != 0) {
		pChr->GetCharacterCore().m_Pos = *MotherPos;
		pChr->GameServer()->CreatePlayerSpawn(*MotherPos);
	}
	else if (players[ClientID]->IsRespawnPointPlaced() && players[ClientID]->GetRespawnPointsNum() >= 1) {
		vec2 NewPos = players[ClientID]->GetRespawnPointPos();
		pChr->GetCharacterCore().m_Pos = NewPos;
		pChr->GameServer()->CreatePlayerSpawn(NewPos);
		players[ClientID]->SetRespawnPointsNum(players[ClientID]->GetRespawnPointsNum() - 1);
	}
	else if (inf_circles.size() > 0) {
		SpawnOnInfZone(pChr);
	}

	// maybe we don't need this
	if (players[ClientID]->GetRespawnPointsNum() <= 0) {
		players[ClientID]->SetRespawnPointPlaced(false);
	}
}

const vec2 *CGameControllerMOD::GetMotherPos() {
	for (CroyaPlayer *each : players)
	{
		if (!each)
			continue;
		if (!each->GetCharacter())
			continue;
		if (each->GetClassNum() == Class::MOTHER && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS)
		{
			return &each->GetCharacter()->GetPos();
		}
	}
	return 0;
}

void CGameControllerMOD::SpawnOnInfZone(CCharacter* pChr) {
	const int TILE_SIZE = 32;
	int RandomInfectionCircleID = random_int_range(0, inf_circles.size() - 1);
	vec2 NewPos = inf_circles[RandomInfectionCircleID]->GetPos();
	int RandomShiftX = random_int_range(-2, 2);	 // todo: make this configurable in .lua (?)
	int RandomShiftY = random_int_range(-1, -3); // todo: make this configurable in .lua (?)
	NewPos.x += RandomShiftX * TILE_SIZE;
	NewPos.y += RandomShiftY * TILE_SIZE;
	pChr->GetCharacterCore().m_Pos = NewPos;
	pChr->GameServer()->CreatePlayerSpawn(NewPos);
} 

int CGameControllerMOD::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	int ClientID = pVictim->GetPlayer()->GetCID();

	players[ClientID]->OnCharacterDeath(pVictim, pKiller, Weapon);
	return 0;
}

bool CGameControllerMOD::IsFriendlyFire(int ClientID1, int ClientID2) const
{
	if (players[ClientID1]->IsHuman() == players[ClientID2]->IsHuman() && ClientID1 != ClientID2)
		return true;
	return false;
}

int CGameControllerMOD::GetIZombieCount() const
{
	int IZombieCount = 0;
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
	if (each->IsZombie() && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS && each->IsInitialZombie()) {
			IZombieCount++;
		}
	}
	return IZombieCount;
}

int CGameControllerMOD::GetZombieCount() const
{
	int ZombieCount = 0;
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (each->IsZombie() && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS) {
			ZombieCount++;
		}
	}
	return ZombieCount;
}

void CGameControllerMOD::ResetHumansToDefault() const
{
  	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		each->ResetRespawnPoint();
		if (each->GetPlayer()->GetTeam() == TEAM_SPECTATORS)
		  continue;
		if (each->IsZombie()) {
		  //each->SetClassNum(each->GetOldClassNum(), true);
		  each->SetClassNum(Class::DEFAULT, false, false, false);
		}
	}
}

int CGameControllerMOD::GetHumanCount() const
{
	int HumanCount = 0;
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (each->IsHuman() && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS) {
			HumanCount++;
		}
	}
	return HumanCount;
}

bool CGameControllerMOD::IsEveryoneInfected() const
{
	bool EveryoneInfected = false;
	int ZombieCount = GetZombieCount();
	if (GetRealPlayerNum() >= 2 && GetRealPlayerNum() == ZombieCount)
		EveryoneInfected = true;
	return EveryoneInfected;
}

void CGameControllerMOD::OnPlayerDisconnect(CPlayer* pPlayer)
{
	IGameController::OnPlayerDisconnect(pPlayer);
	int ClientID = pPlayer->GetCID();

	delete players[ClientID];
	players[ClientID] = nullptr;
}

void CGameControllerMOD::OnPlayerConnect(CPlayer* pPlayer)
{
	IGameController::OnPlayerConnect(pPlayer);
	int ClientID = pPlayer->GetCID();

#ifdef CONF_GEOLOCATION
	char aAddrStr[NETADDR_MAXSTRSIZE];
	Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));
	std::string ip(aAddrStr);
	Server()->SetClientCountry(ClientID, geolocation->get_country_iso_numeric_code(ip));
#endif
	players[ClientID] = new CroyaPlayer(ClientID, pPlayer, GameServer(), this, classes);
	SetLanguageByCountry(Server()->ClientCountry(ClientID), ClientID);
	if (IsCroyaWarmup())
	{
		players[ClientID]->SetClass(classes[Class::DEFAULT]);
	}
	else
	{
		players[ClientID]->TurnIntoRandomZombie();
		players[ClientID]->SetOldClassNum(Class::SCIENTIST); // turn into medic on medic revive
		for (CroyaPlayer *each : players)
		{
			if (each && each->GetPlayer())
			{
				if (each->IsZombie())
				{
					GameServer()->SendClanChange(each->GetClientID(), ClientID, "Zombie");
				}
				else
				{
					GameServer()->SendClanChange(each->GetClientID(), ClientID, "Human");
				}
			}
		}
	}
	GameServer()->SendChatTarget(ClientID, g_Config.m_SvWelcome);
}

std::array<CroyaPlayer*, 64> CGameControllerMOD::GetCroyaPlayers()
{
	return players;
}

void CGameControllerMOD::ResetFinalExplosion()
{
	m_ExplosionStarted = false;

	for (int j = 0; j < m_MapHeight; j++)
	{
		for (int i = 0; i < m_MapWidth; i++)
		{
			if (!(m_GrowingMap[j * m_MapWidth + i] & 4))
			{
				m_GrowingMap[j * m_MapWidth + i] = 1;
			}
		}
	}
}

bool CGameControllerMOD::IsExplosionStarted() const
{
	return m_ExplosionStarted;
}

std::vector<class CCircle*>& CGameControllerMOD::GetCircles()
{
	return circles;
}

std::vector<class CInfCircle*>& CGameControllerMOD::GetInfCircles()
{
	return inf_circles;
}

void CGameControllerMOD::InitTeleporter()
{
	if (!GameServer()->Collision()->Layers()->TeleLayer())
		return;
	int Width = GameServer()->Collision()->Layers()->TeleLayer()->m_Width;
	int Height = GameServer()->Collision()->Layers()->TeleLayer()->m_Height;

	for (int i = 0; i < Width * Height; i++)
	{
		int Number = GameServer()->Collision()->TeleLayer()[i].m_Number;
		int Type = GameServer()->Collision()->TeleLayer()[i].m_Type;
		if (Number > 0)
		{
			if (Type == TILE_TELEOUT)
			{
				m_TeleOuts[Number - 1].push_back(
						vec2(i % Width * 32 + 16, i / Width * 32 + 16));
			}
			else if (Type == TILE_TELECHECKOUT)
			{
				m_TeleCheckOuts[Number - 1].push_back(
						vec2(i % Width * 32 + 16, i / Width * 32 + 16));
			}
		}
	}
}
