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
#include <infcroya/classes/ninja.h>
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
#include <infcroya/localization/localization.h>
#include <yaml-cpp/yaml.h>


CGameControllerMOD::CGameControllerMOD(class CGameContext *pGameServer) :
		IGameController(pGameServer), m_Teams(pGameServer), m_pInitResult(nullptr)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_NAME : GAME_NAME;

	InitTeleporter();

	//IGameController::m_MOD = this; // temporarily, todo: avoid this

	m_ExplosionStarted = false;
	m_InfectedStarted = false;
	m_apFlag = 0;
	safezones.clear();
	inf_circles.clear();

	m_MapWidth = GameServer()->Collision()->GetWidth();
	m_MapHeight = GameServer()->Collision()->GetHeight();
	m_GrowingMap = new int[m_MapWidth * m_MapHeight];

	for (int j = 0; j < m_MapHeight; j++)
	{
		for (int i = 0; i < m_MapWidth; i++)
		{
			vec2 TilePos = vec2(16.0f, 16.0f) + vec2(i * 32.0f, j * 32.0f);
			if (GameServer()->Collision()->CheckPoint(TilePos))
			{
				m_GrowingMap[j * m_MapWidth + i] = 4;
			}
			else
			{
				m_GrowingMap[j * m_MapWidth + i] = 1;
			}
		}
	}

	classes[Class::DEFAULT] = new CDefault();
	classes[Class::BIOLOGIST] = new CBiologist();
	classes[Class::ENGINEER] = new CEngineer();
	classes[Class::MEDIC] = new CMedic();
	classes[Class::SOLDIER] = new CSoldier();
	classes[Class::SCIENTIST] = new CScientist();
	classes[Class::MERCENARY] = new CMercenary();
	classes[Class::NINJA] = new CNinja();
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
  
	m_TimeLimit = g_Config.m_SvTimelimit; // will be updated by a map load
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

			if (Server()->IsSixup(SnappingClient)) {
				protocol7::CNetObj_Pickup* pP = static_cast<protocol7::CNetObj_Pickup*>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, each->GetCharacter()->m_RespawnPointID, sizeof(protocol7::CNetObj_Pickup)));
				if (!pP)
					return;
				pP->m_X = (int)each->GetRespawnPointPos().x;
				pP->m_Y = (int)each->GetRespawnPointPos().y;
				pP->m_Type = POWERUP_ARMOR;

			} else {
				CNetObj_Pickup* pP = static_cast<CNetObj_Pickup*>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, each->GetCharacter()->m_RespawnPointID, sizeof(CNetObj_Pickup)));
				if (!pP)
					return;
				pP->m_X = (int)each->GetRespawnPointPos().x;
				pP->m_Y = (int)each->GetRespawnPointPos().y;
				pP->m_Type = POWERUP_ARMOR;
			}
		}
	}
}

void CGameControllerMOD::OnRoundStart()
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "OnRoundStart");
	const int TILE_SIZE = 32;
	char aBuf[256];

	str_format(aBuf, sizeof(aBuf), "Will read YAML for map: %s", g_Config.m_SvMap);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);

 	std::string path_to_yaml("maps/");
	path_to_yaml += g_Config.m_SvMap;
	path_to_yaml += ".yml";

	str_format(aBuf, sizeof(aBuf), "Loading YAML file: %s", path_to_yaml.c_str());
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);
	try {
		inf_circles.clear();
		YAML::Node mapconfig = YAML::LoadFile(path_to_yaml);
		m_TimeLimit = mapconfig["timelimit"].as<int>();
		const YAML::Node& inf_circle_nodes = mapconfig["inf_circles"];
		for (YAML::const_iterator it = inf_circle_nodes.begin(); it != inf_circle_nodes.end(); ++it) {
			const YAML::Node& inf_circle_node = *it;
			int x = inf_circle_node["x"].as<int>();
			int y = inf_circle_node["y"].as<int>();
			m_GrowingMap[y * m_MapWidth + x] = 6; // final explosion start pos (?)
			int radius = inf_circle_node["radius"].as<int>();
			str_format(aBuf, sizeof(aBuf), "Success parsing inf_circle node");
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);
			inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(x * TILE_SIZE, y * TILE_SIZE), -1, radius));
		}
		flag_positions.clear();
		const YAML::Node& flag_nodes = mapconfig["flags"];
		for (YAML::const_iterator it = flag_nodes.begin(); it != flag_nodes.end(); ++it) {
			const YAML::Node& flag_node = *it;
			int x = flag_node["x"].as<int>();
			int y = flag_node["y"].as<int>();
			str_format(aBuf, sizeof(aBuf), "Success parsing flag node");
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);
			flag_positions.push_back(vec2(x, y));
		}
		const YAML::Node& safezone_nodes = mapconfig["safezones"];
		safezones.clear();
		if ( safezone_nodes.size() > 0) {
			int safezone_num = rand() % safezone_nodes.size();
			str_format(aBuf, sizeof(aBuf), "Safezone spot id %ld of %ld", safezone_num, safezone_nodes.size() - 1);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
			GameServer()->SendChatTarget(-1, aBuf);

			int pos = 0;

			for (YAML::const_iterator it = safezone_nodes.begin(); it != safezone_nodes.end(); ++it) {
				const YAML::Node& safezone_node = *it;
				int x = safezone_node["x"].as<int>() * TILE_SIZE;
				int y = safezone_node["y"].as<int>() * TILE_SIZE;
				float radius = safezone_node["radius"].as<float>();
				float min_radius = safezone_node["min_radius"].as<float>();
				float shrink_speed = safezone_node["shrink_speed"].as<float>();
				str_format(aBuf, sizeof(aBuf), "Success parsing safezone node");
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);
				if (pos == safezone_num)
					safezones.push_back(new CCircle(&GameServer()->m_World, vec2(x, y), -1, radius, min_radius, shrink_speed));
				pos++;
			}
		}

	} catch (const YAML::BadFile&) {
		str_format(aBuf, sizeof(aBuf), "Error parsing YAML file: %s", path_to_yaml.c_str());
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "yaml", aBuf);
		std::exit(1);
	}
	TurnDefaultIntoRandomHuman();
	UnlockPositions();
	ResetTaxi();

	for (CCircle* safezone : safezones) {
		if (safezone->GetRadius() > safezone->GetMinRadius())
			safezone->SetRadius(safezone->GetRadius() - safezone->GetShrinkSpeed());
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
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i]) {
			CPlayer *player = GameServer()->m_apPlayers[i];
			player->m_Score = 0;
		}



}

void CGameControllerMOD::TurnDefaultIntoRandomHuman()
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Turning default to random human TDIRH");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
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

void CGameControllerMOD::ResetTaxi()
{
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (!each->GetCharacter())
			continue;
		each->GetCharacter()->ResetTaxi();
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

//void CGameControllerMOD::Tick()
//{
//	IGameController::Tick();
//	m_Teams.ProcessSaveTeam();
//
//	if(m_pInitResult != nullptr && m_pInitResult.use_count() == 1)
//	{
//		if(m_pInitResult->m_Done)
//		{
//			m_CurrentRecord = m_pInitResult->m_CurrentRecord;
//		}
//		m_pInitResult = nullptr;
//	}
//}
int CGameControllerMOD::GetRealPlayerNum() const {
	int PlayersNum = 0;
	for (CPlayer* each : GameServer()->m_apPlayers) {
		if (!each)
			continue;
		if (!each->GetCroyaPlayer())
			continue;
		if(!each->GetTeam() == 0)
			continue;
		PlayersNum++;
	}
	return (int) PlayersNum;
}

bool CGameControllerMOD::IsGamePhase() {
	if (IsCroyaWarmup()) // we're in warmup
	  return false;
	if (!m_InfectedStarted) // no game has started
	  return false;
    if (IsGameEnd()) // game is ended, scroreboard is on screen
	  return false;
	return true;
}

bool CGameControllerMOD::WarmupJustended() {
	if (IsCroyaWarmup()) // we're in warmup
	  return false;
    if (IsGameEnd()) // game is ended, scroreboard is on screen
	  return false;
	if (!m_InfectedStarted) // warmup didn't started
	  return false;
    return true;
}

bool CGameControllerMOD::ShouldDoWarmup() {
	if (IsCroyaWarmup()) // we're in warmup already
	  return false;
	if (m_InfectedStarted) // warmup is ended, we have a game
	  return false;
    if (IsGameEnd()) // game is ended, scroreboard is on screen
	  return false;
	if (GetRealPlayerNum() < 2)
	  return false;
    return true;
}

bool CGameControllerMOD::ShouldEndGame() {
	if (!m_InfectedStarted) // no game or warmup
	  return false;
    if (IsGameEnd()) // game is alrady ended, scroreboard is on screen
	  return false;
	if (IsEveryoneInfected()) // normal game end, humans lose
	  return true;
	if (GetRealPlayerNum() < 2) // someone leaved or gone spectating
	  return true;
    return false;
}

void CGameControllerMOD::DoInfectedWon() {
	int Seconds = (Server()->Tick() - m_GameStartTick) / ((float)Server()->TickSpeed());
	for (CPlayer* each : GameServer()->m_apPlayers) {
		if (!each)
			continue;
		if (!each->GetCroyaPlayer())
			continue;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), localize("Infected won the round in %d seconds", each->GetCroyaPlayer()->GetLanguage()).c_str(), Seconds);
		GameServer()->SendChatTarget(each->GetCID(), aBuf);
	}
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "Everyone Infected");
}

bool CGameControllerMOD::ShouldDoFinalExplosion() {
	if (m_ExplosionStarted || GameServer()->m_World.m_Paused)
	  return false;
	int Seconds = (Server()->Tick() - m_GameStartTick) / ((float)Server()->TickSpeed());
	if (m_InfectedStarted && !m_ExplosionStarted && m_TimeLimit > 0 && Seconds >= m_TimeLimit * 60)
	{
		for (CCharacter* p = (CCharacter*)GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter*)p->TypeNext())
		{
			if (p->IsZombie())
			{
				GameServer()->SendEmoticon(p->GetPlayer()->GetCID(), EMOTICON_GHOST);
			}
			else
			{
				GameServer()->SendEmoticon(p->GetPlayer()->GetCID(), EMOTICON_EYES);
			}
		}
		return true;
	}
	return false;
}

void CGameControllerMOD::DoFinalExplosion() {
	bool NewExplosion = false;

	for (int j = 0; j < m_MapHeight; j++)
	{
		for (int i = 0; i < m_MapWidth; i++)
		{
			if ((m_GrowingMap[j * m_MapWidth + i] & 1) && (
				(i > 0 && m_GrowingMap[j * m_MapWidth + i - 1] & 2) ||
				(i < m_MapWidth - 1 && m_GrowingMap[j * m_MapWidth + i + 1] & 2) ||
				(j > 0 && m_GrowingMap[(j - 1) * m_MapWidth + i] & 2) ||
				(j < m_MapHeight - 1 && m_GrowingMap[(j + 1) * m_MapWidth + i] & 2)
				))
			{
				NewExplosion = true;
				m_GrowingMap[j * m_MapWidth + i] |= 8;
				m_GrowingMap[j * m_MapWidth + i] &= ~1;
				if (random_prob(0.1f))
				{
					vec2 TilePos = vec2(16.0f, 16.0f) + vec2(i * 32.0f, j * 32.0f);
					GameServer()->CreateExplosion(TilePos, -1, WEAPON_GAME, 0);
					GameServer()->CreateSound(TilePos, SOUND_GRENADE_EXPLODE);
				}
			}
		}
	}

	for (int j = 0; j < m_MapHeight; j++)
	{
		for (int i = 0; i < m_MapWidth; i++)
		{
			if (m_GrowingMap[j * m_MapWidth + i] & 8)
			{
				m_GrowingMap[j * m_MapWidth + i] &= ~8;
				m_GrowingMap[j * m_MapWidth + i] |= 2;
			}
		}
	}

	for (CCharacter* p = (CCharacter*)GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter*)p->TypeNext())
	{
		if (p->IsHuman())
			continue;

		int tileX = static_cast<int>(round(p->GetPos().x)) / 32;
		int tileY = static_cast<int>(round(p->GetPos().y)) / 32;

		if (tileX < 0) tileX = 0;
		if (tileX >= m_MapWidth) tileX = m_MapWidth - 1;
		if (tileY < 0) tileY = 0;
		if (tileY >= m_MapHeight) tileY = m_MapHeight - 1;

		if (m_GrowingMap[tileY * m_MapWidth + tileX] & 2 && p->GetPlayer())
		{
			p->Die(p->GetPlayer()->GetCID(), WEAPON_GAME);
		}
	}

	//If no more explosions, game over, decide who win
	if (!NewExplosion)
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "No more explosions");
		if (GameServer()->GetHumanCount())
		{
			int NumHumans = GameServer()->GetHumanCount();

			for (CPlayer* each : GameServer()->m_apPlayers) {
				if (!each)
					continue;
				if (!each->GetCroyaPlayer())
					continue;

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), localize("%d humans won the round", each->GetCroyaPlayer()->GetLanguage()).c_str(), NumHumans);
				GameServer()->SendChatTarget(each->GetCID(), aBuf);
				if (each->GetCroyaPlayer()->IsHuman())
				{
					GameServer()->SendChatTarget(each->GetCID(), localize("You have survived!", each->GetCroyaPlayer()->GetLanguage()).c_str());
					each->m_Score += 5;
				}
			}
		}
		else
		{
			int Seconds = m_TimeLimit * 60;
			for (CPlayer* each : GameServer()->m_apPlayers) {
				if (!each)
					continue;
				if (!each->GetCroyaPlayer())
					continue;
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), localize("Infected won the round in %d seconds at the last moment", each->GetCroyaPlayer()->GetLanguage()).c_str(), Seconds);
				GameServer()->SendChatTarget(each->GetCID(), aBuf);
			}
		}
		OnRoundEnd();
	}
}

void CGameControllerMOD::Tick()
{
	IGameController::Tick(); // process warmup and endgame timers to start round

	if (RoundJustStarted()) { // executed twice: 1. after map start 2. after warmup end
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "RoundJustStarted");
		if (WarmupJustended()) {
			m_GameStartTick = Server()->Tick();
			OnRoundStart(); // draw circles and such only after a warmup
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "Initial infection first");
			TurnDefaultIntoRandomHuman();
			StartInitialInfection();
		}
	}

	if (ShouldDoWarmup()) {
		ResetZombiesToDefaultHumans();
		m_GameStartTick = Server()->Tick();
		DoWarmup(10);
		m_InfectedStarted = true;
		for (CroyaPlayer* each : players) {
			if (!each)
				continue;
			if (each->GetPlayer()->GetTeam() == TEAM_SPECTATORS)
				continue;
			GameServer()->SendClassSelectorByClassId(each->GetClassNum(), each->GetClientID(), true);
		}
	}

	if (ShouldEndGame()) { // 1. no humans survived 2. less than 2 players in game
		DoInfectedWon();
		OnRoundEnd();
	}

	if (IsGamePhase()) {
		for (auto each : players)
		{
			if (!each)
				continue;

			each->Tick();
		}

		FlagTick();

		for (CCircle* safezone : safezones) {
			if (safezone->GetRadius() > safezone->GetMinRadius())
				safezone->SetRadius(safezone->GetRadius() - safezone->GetShrinkSpeed());
		}

		// no zombies, start infection
		if (GetRealPlayerNum() >= 2 && GetZombieCount() < 1)
		{
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "Initial infection midgame");
			StartInitialInfection();
		}
	}

	//Start the final explosion if the time is over
	if (ShouldDoFinalExplosion() && !m_ExplosionStarted && !GameServer()->m_World.m_Paused) {
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "ShouldDoFinalExplosion");
		m_ExplosionStarted = true;
	}

	//Do the final explosion
	if (m_ExplosionStarted) {
		DoFinalExplosion();
	}

}

void CGameControllerMOD::FlagTick() {
	if (m_apFlag == 0)
		return;
	
	for (auto each : players)
	{
		if (!each)
			continue;

		if (!each->GetClassNum())
			continue;

		CCharacter *apCloseCCharacters[MAX_CLIENTS];

		int Num = GameServer()->m_World.FindEntities(m_apFlag->GetPos(), CFlag::ms_PhysSize, (CEntity **)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		char aBuf[256];

		for (int i = 0; i < Num; i++)
		{
			if (!apCloseCCharacters[i]->IsAlive())
				// || GameServer()->Collision()->IntersectLine(F->GetPos(), apCloseCCharacters[i]->GetPos(), NULL, NULL))
				continue;

			if (! apCloseCCharacters[i]->IsHuman())
				continue;
				
			if (apCloseCCharacters[i]->GetCroyaPlayer()->GetClassNum() != Class::HERO)
			    continue;

			str_format(aBuf, sizeof(aBuf), "Debug1 flag is taken");
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);

			apCloseCCharacters[i]->GetPlayer()->m_Score += 1;

			apCloseCCharacters[i]->GiveWeapon(WEAPON_GRENADE, 10);

			str_format(aBuf, sizeof(aBuf), "Flag is taken by a hero! Humans +4 HP/shelds");
			GameServer()->CreateSound(m_apFlag->GetPos(), SOUND_CTF_CAPTURE);
			GameServer()->SendChatTarget(-1, aBuf);
			str_format(aBuf, sizeof(aBuf), "Debug1 trying respawn flag");
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
			if (flag_positions.size() > 0)
			{
				str_format(aBuf, sizeof(aBuf), "Debug1 flag_positions.size > 0");
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
				//int randomIndex = rand() % flag_positions.size();
				//vec2 newPos = flag_positions[randomIndex];
				m_apFlagSpotId++;
				str_format(aBuf, sizeof(aBuf), "Debug1 m_apFlagSpotId will be %ld?", m_apFlagSpotId);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);

				if (m_apFlagSpotId >= flag_positions.size())
					m_apFlagSpotId = 0;
				str_format(aBuf, sizeof(aBuf), "Debug1 m_apFlagSpotId will be %ld!", m_apFlagSpotId);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
				str_format(aBuf, sizeof(aBuf), "Flag spot id %ld of %ld", m_apFlagSpotId, flag_positions.size() - 1);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
				GameServer()->SendChatTarget(-1, aBuf);

				vec2 newPos = flag_positions[m_apFlagSpotId];
				str_format(aBuf, sizeof(aBuf), "Debug1 got newpos");
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
				str_format(aBuf, sizeof(aBuf), "Flag is spawned on %f, %f", newPos.x, newPos.y);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);

				m_apFlag->SetPos(newPos);
				str_format(aBuf, sizeof(aBuf), "New flag is spawned");
				GameServer()->SendChatTarget(-1, aBuf);

				for (auto each : players)
				{
					if (!each || each->IsZombie())
						continue;
					if (!each->GetCharacter() == 0)
					  each->GetCharacter()->IncreaseOverallHp(4);
				}
				str_format(aBuf, sizeof(aBuf), "Debug1 health increased");	
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
			}
		}
	}
}

bool CGameControllerMOD::IsCroyaWarmup()
{
	if (IsWarmup())
		return true;

/* 	if (m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) <= Server()->TickSpeed() * 10)
		return true;
	else */
		return false;
}

bool CGameControllerMOD::RoundJustStarted()
{
	if (m_RoundStartTick + 1 == Server()->Tick()) // to avoid reset
		return true;
	else
		return false;
}

void CGameControllerMOD::StartInitialInfection()
{
	for (CroyaPlayer* each : players) {
		if (!each)
			continue;
		if (each->IsHuman() && each->GetPlayer()->GetTeam() != TEAM_SPECTATORS)
			humans.push_back(each->GetClientID());
	}
	if (GetRealPlayerNum() < 2) {
		humans.clear();
		return;
	}

	auto infect_random_human = [this](int n) {
		for (int i = 0; i < n; i++) {
			int HumanVecID = random_int_range(0, humans.size() - 1);
			int RandomHumanID = humans[HumanVecID];
			players[RandomHumanID]->SetOldClassNum(players[RandomHumanID]->GetClassNum());
			players[RandomHumanID]->TurnIntoRandomZombie();
			humans.erase(humans.begin() + HumanVecID);
			CroyaPlayer* each = players[RandomHumanID];
			GameServer()->SendClassSelectorByClassId(each->GetClassNum(), each->GetClientID(), true);
		}
	};

	if (GetRealPlayerNum() <= 3 && GetZombieCount() < 1) {
		infect_random_human(1);
	}
	else if (GetRealPlayerNum() > 3 && GetZombieCount() < 2) {
		infect_random_human(2);
	}
	
	humans.clear();
}

void CGameControllerMOD::OnRoundEnd()
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "OnRoundEnd");
	ResetFinalExplosion();
	safezones.clear();
	inf_circles.clear();
    if (m_apFlag != 0) {
		m_apFlag->Destroy();
		m_apFlag = 0;
	}
	m_InfectedStarted = false;
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

	if (pChr->IsZombie()) {
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
	for (CPlayer* each : GameServer()->m_apPlayers) {
		if (!each)
			continue;
		if (!each->GetCroyaPlayer())
			continue;
		if(!each->GetTeam() == 0)
			continue;
		if (!each->GetCroyaPlayer()->IsZombie())
			continue;
		ZombieCount++;
	}
	return ZombieCount;
}

void CGameControllerMOD::ResetZombiesToDefaultHumans() const
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Turning zombies back to default humans RHTD");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
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
	//TBDTBD
	//IGameController::OnPlayerDisconnect(pPlayer);
	//TBD
	int ClientID = pPlayer->GetCID();

	delete players[ClientID];
	players[ClientID] = nullptr;
}

void CGameControllerMOD::OnPlayerConnect(CPlayer* pPlayer)
{
	//TBDTBD
	//IGameController::OnPlayerConnect(pPlayer);
	//TBD
	int ClientID = pPlayer->GetCID();

#ifdef CONF_GEOLOCATION
	char aAddrStr[NETADDR_MAXSTRSIZE];
	Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));
	std::string ip(aAddrStr);
	Server()->SetClientCountry(ClientID, geolocation->get_country_iso_numeric_code(ip));
#endif
	players[ClientID] = new CroyaPlayer(ClientID, pPlayer, GameServer(), this, classes);
	//SetLanguageByCountry(Server()->ClientCountry(ClientID), ClientID);
	//TBD
	if (IsCroyaWarmup() || !m_InfectedStarted)
	{
		players[ClientID]->SetClass(classes[Class::DEFAULT]);
	}
	else
	{
		players[ClientID]->TurnIntoRandomZombie();
		players[ClientID]->SetOldClassNum(Class::SCIENTIST); // turn into it on medic revive
/* 		for (CroyaPlayer *each : players)
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
		} */
	}
	//GameServer()->SendChatTarget(ClientID, g_Config.m_SvWelcome);
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

std::vector<class CCircle*>& CGameControllerMOD::GetSafezones()
{
	return safezones;
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

