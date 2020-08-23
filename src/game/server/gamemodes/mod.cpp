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
#include <infcroya/localization/localization.h>


CGameControllerMOD::CGameControllerMOD(class CGameContext *pGameServer) :
		IGameController(pGameServer), m_Teams(pGameServer), m_pInitResult(nullptr)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_NAME : GAME_NAME;

	InitTeleporter();

	//IGameController::m_MOD = this; // temporarily, todo: avoid this
	//lua = nullptr;
	//TBD

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
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "OnRoundStart");
	const int TILE_SIZE = 32;
	char aBuf[256];
	//if (!lua) { // not sure yet if OnRoundStart is run only once
	//	std::string path_to_lua("maps/");
	//	path_to_lua += g_Config.m_SvMap;
	//	path_to_lua += ".lua";
	//	//lua = new LuaLoader(GameServer());
	//	//lua->Load(path_to_lua.c_str());
	//	//lua->Init(GetRealPlayerNum());

	//	//// positions.size() should be equal to radiuses.size()
	//	//// safezone circles
	//	//flag_positions = lua->GetFlagsPositions();

	//	//auto positions = lua->GetCirclePositions();
	//	//auto radiuses = lua->GetCircleRadiuses();
	//	//auto min_radiuses = lua->GetCircleMinRadiuses();
	//	//auto shrink_speeds = lua->GetCircleShrinkSpeeds();
	//	const int TILE_SIZE = 32;

	//	//for (size_t i = 0; i < positions.size(); i++) {
	//	//	int x = positions[i].x * TILE_SIZE;
	//	//	int y = positions[i].y * TILE_SIZE;
	//	//	circles.push_back(new CCircle(&GameServer()->m_World, vec2(x, y), -1, radiuses[i], min_radiuses[i], shrink_speeds[i]));
	//	//}

	//	// infection zone circles
	//	//auto inf_positions = lua->GetInfCirclePositions();
	//	//auto inf_radiuses = lua->GetInfCircleRadiuses();
	//	//m_GrowingMap[(int)inf_positions[0].y * m_MapWidth + (int)inf_positions[0].x] = 6; // final explosion start pos (?)
	//	//for (size_t i = 0; i < inf_positions.size(); i++) {
	//	//	const int TILE_SIZE = 32;
	//	//	int x = inf_positions[i].x * TILE_SIZE;
	//	//	int y = inf_positions[i].y * TILE_SIZE;
	//	//	inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(x, y), -1, inf_radiuses[i]));
	//	//}
	//	//////inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(30, 30), -1, 100));
	//}
	inf_circles.push_back(new CInfCircle(&GameServer()->m_World, vec2(44 * TILE_SIZE, 30 * TILE_SIZE), -1, 100));
	m_InfectedStarted = true;
	TurnDefaultIntoRandomHuman();
	UnlockPositions();

	//for (CCircle* circle : circles) {
	//	if (circle->GetRadius() > circle->GetMinRadius())
	//		circle->SetRadius(circle->GetRadius() - circle->GetShrinkSpeed());
	//}

	flag_positions.push_back(vec2(44 * TILE_SIZE, 30 * TILE_SIZE));
	flag_positions.push_back(vec2(84 * TILE_SIZE, 30 * TILE_SIZE));
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
/* 	for (auto each : players)
	{
		if (!each)
			continue;
		if(!each->GetPlayer())
			continue;

		PlayersNum++;
	} */
	return (int) PlayersNum;
}

void CGameControllerMOD::ShouldStartTheGame() {}
}

void CGameControllerMOD::ShouldDoWarmup() {}
}

void CGameControllerMOD::ShouldDoWarmup() {}
}

void CGameControllerMOD::Tick()
{
	IGameController::Tick();

	if (RoundJustStarted()) { // not sure if it is executed only once
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "RoundJustStarted");
		OnRoundStart();
	}

	// everyone infected -> end round
	if (!IsCroyaWarmup() && IsEveryoneInfected() && !IsGameEnd() && m_InfectedStarted) {
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
		OnRoundEnd();
	}

	bool IsGameStarted = !IsCroyaWarmup() && !IsGameEnd();

	// end round on 1 player
	if (IsGameStarted && GetRealPlayerNum() < 2 && m_InfectedStarted) {
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "<2 players");
		OnRoundEnd();
	}

	if (!IsGameEnd() && !IsWarmup() && !m_InfectedStarted) {
		if (GetRealPlayerNum() > 1) {
			m_InfectedStarted = true;
			ResetHumansToDefault();
			DoWarmup(10);
		}
	}

	IsGameStarted = !IsCroyaWarmup() && !IsGameEnd();

	if (IsGameStarted && m_InfectedStarted) {
		for (auto each : players)
		{
			if (!each)
				continue;

			each->Tick();
		}

		FlagTick();

		for (CCircle* circle : circles) {
			if (circle->GetRadius() > circle->GetMinRadius())
				circle->SetRadius(circle->GetRadius() - circle->GetShrinkSpeed());
		}

		// no zombies, start infection
		if (GetRealPlayerNum() >= 2 && GetZombieCount() < 1)
		{
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "Initial infection");
			StartInitialInfection();
		}
	}

	// FINAL EXPLOSION BEGIN, todo: write a function for this?
	//infclass 0.6 copypaste
	//Start the final explosion if the time is over
	if (m_InfectedStarted && !m_ExplosionStarted && g_Config.m_SvTimelimit > 0 && (Server()->Tick() - m_GameStartTick) >= g_Config.m_SvTimelimit * Server()->TickSpeed() * 60)
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
		m_ExplosionStarted = true;
	}

	//Do the final explosion
	if (m_ExplosionStarted)
	{
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
						GameServer()->SendChatTarget(each->GetCID(), localize("You have survived, +5 points", each->GetCroyaPlayer()->GetLanguage()).c_str());
						each->m_Score += 5;
					}
				}
			}
			else
			{
				int Seconds = g_Config.m_SvTimelimit * 60;
				for (CPlayer* each : GameServer()->m_apPlayers) {
					if (!each)
						continue;
					if (!each->GetCroyaPlayer())
						continue;
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), localize("Infected won the round in %d seconds", each->GetCroyaPlayer()->GetLanguage()).c_str(), Seconds);
					GameServer()->SendChatTarget(each->GetCID(), aBuf);
				}
			}
			OnRoundEnd();
		}
	}
	// FINAL EXPLOSION END
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

	if (m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) <= Server()->TickSpeed() * 10)
		return true;
	else
		return false;
}

bool CGameControllerMOD::RoundJustStarted()
{
	//if (!IsWarmup() && m_RoundStartTick + 1 == Server()->Tick()) // it will reset so we should wait 1 tick
	if (!m_InfectedStarted)
		return false;
	if (!IsWarmup() && m_RoundStartTick + 1 == Server()->Tick()) // it will reset so we should wait 1 tick
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
	if (!m_InfectedStarted)
	  return;
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", "OnRoundEnd");
	//m_InfectedStarted = false; TBD
	ResetFinalExplosion();
	ResetHumansToDefault();
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
	//IGameController::OnPlayerDisconnect(pPlayer);
	//TBD
	int ClientID = pPlayer->GetCID();

	delete players[ClientID];
	players[ClientID] = nullptr;
}

void CGameControllerMOD::OnPlayerConnect(CPlayer* pPlayer)
{
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

