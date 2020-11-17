#include "croyaplayer.h"
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>
#include <infcroya/classes/class.h>
#include <infcroya/localization/localization.h>
#include <infcroya/entities/circle.h>
#include <infcroya/entities/inf-circle.h>
#include <game/server/gamemodes/mod.h>
#include <game/version.h>
#include <limits>

CroyaPlayer::CroyaPlayer(int ClientID, CPlayer* pPlayer, CGameContext* pGameServer, CGameControllerMOD* pGameController, std::unordered_map<int, IClass*> Classes)
{
	m_ClientID = ClientID;
	m_pPlayer = pPlayer;
	m_pPlayer->SetCroyaPlayer(this);
	m_pGameServer = pGameServer;
	m_pGameController = pGameController;
	m_pCharacter = nullptr;
	m_Infected = false;
	m_HookProtected = false;
	m_Classes = Classes;
	m_Language = "english";

	m_RespawnPointsNum = 1;
	m_RespawnPointsDefaultNum = 1;
	m_RespawnPointPlaced = false;
	m_RespawnPointPos = vec2(0, 0);
	m_RespawnPointDefaultCooldown = 3;
	m_RespawnPointCooldown = 0;
	m_AirJumpCounter = 0;
	m_CanLockPosition = true;
	m_IsPositionLocked = false;
	m_IsAntigravityOn = false;
	m_InsideInfectionZone = false;
	m_InsideSafeZone = true;
	m_BeenOnRoundStart = false;
	m_InitialZombie = false;
	m_MedicHeals = 0;
	SetClass(Classes[DEFAULT], false, false);
}

CroyaPlayer::~CroyaPlayer()
{
}

void CroyaPlayer::SetBeenOnRoundStart() {
	m_BeenOnRoundStart = true;
}

void CroyaPlayer::Tick() // todo cleanup INF circles and safezones are mixed
{
	SetInsideInfectionZone(false);

	if (m_pCharacter && m_pClass && m_pCharacter->GameWorld()) { // so many null checks, not sure which are necessary
		m_pClass->Tick(m_pCharacter);
	}

	if (IsHuman() && m_pCharacter && m_pCharacter->GameWorld()) {
		// Infect when inside infection zone circle
		auto circle = GetClosestInfCircle();
		if (circle) {
			float dist = distance(m_pCharacter->GetPos(), circle->GetPos());
			if (dist < circle->GetRadius()) {
				m_pCharacter->Infect(-1);
				m_pGameServer->SendBroadcast("You've fallen into an infection circle!", m_pPlayer->GetCID());
			}
		}

		// Take damage when outside of safezone circle
		if (!m_InsideSafeZone) {
			int Dmg = 1;
			if (m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0)
			{ // each second
				if (m_pCharacter->GetCroyaPlayer()->GetClassNum() == Class::HERO)
				{
					if (m_pGameServer->Server()->Tick() % (m_pGameServer->Server()->TickSpeed() * 3) == 0) {
						m_pCharacter->TakeDamage(vec2(0, 0), m_pCharacter->GetPos(), Dmg, m_ClientID, WEAPON_WORLD);
						m_pGameServer->SendBroadcast("You've stepped outside of the safezone!", m_pPlayer->GetCID());
					}
				} else {
					m_pCharacter->TakeDamage(vec2(0, 0), m_pCharacter->GetPos(), Dmg, m_ClientID, WEAPON_WORLD);
					m_pGameServer->SendBroadcast("You've stepped outside of the safezone!", m_pPlayer->GetCID());
				}
			}
		}
	}

	if (IsZombie() && m_pCharacter) {
		// Increase health when inside infection zone circle
		auto inf_circle = GetClosestInfCircle();
		if (inf_circle) {
			float dist = distance(m_pCharacter->GetPos(), inf_circle->GetPos());
			if (dist < inf_circle->GetRadius() && m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0) { // each second
				m_pCharacter->IncreaseHealth(2);
				m_RespawnPointsNum = m_RespawnPointsDefaultNum;
			}
			if (dist < inf_circle->GetRadius()) {
				SetInsideInfectionZone(true);
			}
		}
	}

	if (m_RespawnPointCooldown > 0) {
		if (m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0) { // each second
			m_RespawnPointCooldown -= 1;
		}
	}

	if (IsHuman() && m_pCharacter && m_pCharacter->GameWorld() && m_pCharacter->GetCharacterCore().m_HookedPlayer >=0) {
		CCharacter* VictimChar = m_pGameServer->GetPlayerChar(m_pCharacter->GetCharacterCore().m_HookedPlayer);
		if (VictimChar && VictimChar->IsHuman()) {
			if (VictimChar->IsInSlowMotion()) {
				VictimChar->m_IsInSlowMotion = false;
				if (VictimChar->GetCroyaPlayer()) {
					for(int i = 0; i < MAX_CLIENTS; i++) // linear search, better would be a stored attribute of hooker
					{
						CCharacterCore *pCharCore = GetCharacter()->GetCharacterCore().m_pWorld->m_apCharacters[i];
						if (!pCharCore || pCharCore == GetCharacter()->GetpCore())
							continue;
						if (pCharCore->m_HookedPlayer == -1 || pCharCore->m_HookedPlayer != VictimChar->GetCroyaPlayer()->GetClientID())
							continue;
						pCharCore->m_HookedPlayer = -1;
						pCharCore->m_HookState = HOOK_RETRACTED;
						pCharCore->m_HookPos = pCharCore->m_Pos;
					}
				}
			}
		}
	}
	if ((IsZombie() || GetClassNum() == Class::PSYCHO) && m_pCharacter && m_pCharacter->GameWorld()) {
		if (m_pCharacter->GetCharacterCore().m_HookedPlayer >= 0) {
			CCharacter* VictimChar = m_pGameServer->GetPlayerChar(m_pCharacter->GetCharacterCore().m_HookedPlayer);
			if (VictimChar)
			{
				float Rate = 1.0f;
				int Damage = 1;

				if (GetClassNum() == Class::SMOKER)
				{
					Rate = 1.0f;
					Damage = g_Config.m_InfSmokerHookDamage;
				}

				if (GetClassNum() == Class::MOTHER)
				{
					Rate = 1.0f;
					Damage = 0;
				}

				if (VictimChar->GetCroyaPlayer()->GetClassNum() == Class::PSYCHO) {
					Rate = 1.0;
				}

				if (GetClassNum() == Class::SPIDER)
				{
					Rate = 3.0f;
					Damage = 1;
					if (VictimChar->IsHuman()) {
						VictimChar->RemoveNinja();
						//VictimChar->Core()->m_Vel = vec2(0, 0);
						if (!VictimChar->IsInSlowMotion())
						{
							VictimChar->SlowMotionEffect(1.0f);
						}
						//VictimChar->SlowMotionEffect(5.0f);
					}
				}			

				if (GetClassNum() == Class::FREEZER)
				{
					Rate = 1.0f;
					Damage = 0;
					if (VictimChar->IsHuman()) {
						//VictimChar->Freeze(1.0f, m_ClientID, FREEZEREASON_FLASH);
						VictimChar->Stun(0.4f, 4);
						//VictimChar->Die(m_ClientID, WEAPON_HAMMER);
					}
				}			

				if (GetClassNum() == Class::PARASITE){
					Rate = 1.0f;
					Damage = 0;
					if (VictimChar->IsHuman()) {
						//const CSkin skin = VictimChar->GetCroyaPlayer()->GetClass()->GetSkin();
						//CPlayer *pPlayer = GetPlayer();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_BODY], skin.GetBodyName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_BODY] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_BODY] = skin.GetBodyColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_MARKING], skin.GetMarkingName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_MARKING] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_MARKING] = skin.GetMarkingColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_DECORATION], skin.GetDecorationName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_DECORATION] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_DECORATION] = skin.GetDecorationColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_HANDS], skin.GetHandsName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_HANDS] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_HANDS] = skin.GetHandsColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_FEET], skin.GetFeetName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_FEET] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_FEET] = skin.GetFeetColor();

						//if (VictimChar->GetCroyaPlayer() && (
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::SNIPER ||
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::MEDIC ||
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::SOLDIER )
						//) {
				  //      	str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_EYES], "negative", 24);
				  //          pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = 130944;
						//} else {
				  //      	str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_EYES], "negative", 24);
				  //          pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = 128104;
						//}
				  //      pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_EYES] = true;
				  //      //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = skin.GetEyesColor();

						//m_pGameServer->SendAllClientsSkinChange(GetClientID());
					}
				}

				if (m_pCharacter->m_HookDmgTick + m_pGameServer->Server()->TickSpeed() * Rate < m_pGameServer->Server()->Tick())
				{
					m_pCharacter->m_HookDmgTick = m_pGameServer->Server()->Tick();
					if (Damage > 0)
					  VictimChar->TakeDamage(vec2(0.0f, 0.0f), m_pCharacter->GetPos(), Damage, m_pPlayer->GetCID(), WEAPON_NINJA);
					if (GetClassNum() == Class::SMOKER && !VictimChar->IsZombie()) {
						m_pCharacter->IncreaseOverallHp(Damage); // blood licking
					}
					if (GetClassNum() == Class::PSYCHO && VictimChar->IsZombie()) {
						m_pCharacter->IncreaseHealth(1); // blood licking
					}
				}
			}
		}
	}
	
	// set this at the end of CroyaPlayer tick so entities could overwrite this
	if (IsHuman()) // we don't care about zombies
		SetInsideSafeZone(IsInsideSafeZone());

}

bool CroyaPlayer::IsInsideSafeZone() {
	auto safe_circle = GetClosestCircle(); // closest only
	if (safe_circle)
	{
		float dist = distance(m_pCharacter->GetPos(), safe_circle->GetPos());
		if (dist > safe_circle->GetRadius())
			return false;
	}
	return true;
}

CCircle* CroyaPlayer::GetClosestCircle()
{
	float min_distance = std::numeric_limits<float>::max();
	CCircle* closest = nullptr;
	for (CCircle* circle : m_pGameController->GetSafezones()) {
		if (!m_pCharacter)
			break;
		if (!circle)
			continue; // maybe unnecessary, not sure

		float dist = distance(m_pCharacter->GetPos(), circle->GetPos()) - circle->GetRadius();
		if (dist < min_distance) {
			min_distance = dist;
			closest = circle;
		}
	}
	if (closest) {
		return closest;
	}
	return nullptr;
}

CInfCircle* CroyaPlayer::GetClosestInfCircle()
{
	float min_distance = std::numeric_limits<float>::max();
	CInfCircle* closest = nullptr;
	for (CInfCircle* circle : m_pGameController->GetInfCircles()) {
		if (!m_pCharacter)
			break;
		if (!circle)
			continue; // maybe unnecessary, not sure

		float dist = distance(m_pCharacter->GetPos(), circle->GetPos()) - circle->GetRadius();
		if (dist < min_distance) {
			min_distance = dist;
			closest = circle;
		}
	}
	if (closest) {
		return closest;
	}
	return nullptr;
}

int CroyaPlayer::GetClassNum()
{
	int ClassNum;
	for (const auto& c : m_Classes) {
		if (m_pClass == c.second) {
			ClassNum = c.first;
			break;
		}
	}
	return ClassNum;
}

void CroyaPlayer::SetClassNum(int Class, bool DrawPurpleThing, bool ShowInfo, bool destroyChildEntities)
{
	if (GetCharacter())
		GetCharacter()->ResetTaxi();
	// char aBuf[512];
	// str_format(aBuf, sizeof(aBuf),
	// 		"SetClassNum, Client ID = %d, class = %d'",
	// 		m_ClientID, Class);
	// m_pGameServer->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
	SetClass(m_Classes[Class], DrawPurpleThing, destroyChildEntities);
	m_pGameServer->SendClassSelectorByClassId(Class, GetClientID(), ShowInfo);
}

bool CroyaPlayer::SetZombieClassNumPlease(int Class, bool DrawPurpleThing, bool Force)
{
	if (!Force) {
		if (!IsZombie())
			return false;

		if (!IsInsideInfectionZone()) {
			return false;
		}
	}

	SetClassNum(Class, DrawPurpleThing);
	ResetRespawnPoint();
	return true;
}

bool CroyaPlayer::SetHumanClassNumPlease(int Class, bool DrawPurpleThing, bool Force)
{
	if (!Force) {
		if (!m_pGameController->IsCroyaWarmup())
		return false;
	}

	SetClassNum(Class, DrawPurpleThing, true);
	return true;
}

CCharacter* CroyaPlayer::GetCharacter() {
	return m_pCharacter;
}

void CroyaPlayer::SetCharacter(CCharacter* pCharacter)
{
	m_pCharacter = pCharacter;
}

CPlayer* CroyaPlayer::GetPlayer()
{
	return m_pPlayer;
}

int CroyaPlayer::GetClientID() const
{
	return m_ClientID;
}

void CroyaPlayer::OnCharacterSpawn(CCharacter* pChr)
{
	m_MedicHeals = 0;
	m_pClass->OnCharacterSpawn(pChr);
	m_pCharacter->SetCroyaPlayer(this);
	m_pCharacter->SetHookProtected(m_HookProtected);
	if (m_BeenOnRoundStart) {
		dbg_msg("game", "set armor on character span croya as was on round start");
		GetCharacter()->SetArmor(10);
		m_BeenOnRoundStart = false;
	}
}

void CroyaPlayer::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	m_pClass->OnCharacterDeath(pVictim, pKiller, Weapon);
	if (IsHuman()) {
		SetOldClassNum(GetClassNum());
		TurnIntoRandomZombie();
	}
	m_pCharacter = nullptr;
}

void CroyaPlayer::OnKill(int Victim)
{
	int64 Mask = CmaskOne(m_ClientID);
	m_pGameServer->CreateSound(m_pPlayer->m_ViewPos, SOUND_CTF_GRAB_PL, Mask);

	if (Victim == m_ClientID)
	  return;

	if (IsZombie()) {
		m_pPlayer->m_Score += 3;
		m_pPlayer->m_Infections += 1;
	}
	else {
		m_pPlayer->m_Kills += 1;
		m_pPlayer->m_Score++;
	}
}

bool CroyaPlayer::WillItFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	m_pCharacter = pChr;
	return m_pClass->WillItFire(Direction, ProjStartPos, Weapon, m_pCharacter);
}

void CroyaPlayer::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	m_pCharacter = pChr;
	m_pClass->OnWeaponFire(Direction, ProjStartPos, Weapon, m_pCharacter);
}

void CroyaPlayer::OnButtonF3()
{
	SetHookProtected(!IsHookProtected());
}

void CroyaPlayer::SetSpawnPointAt(vec2 NewPos)
{
	if (IsHuman()) {
		return;
	}

	if (!m_pGameController->IsCroyaWarmup()) {
		m_RespawnPointPos = NewPos;
		m_RespawnPointPlaced = true;
	}
}

void CroyaPlayer::OnMouseWheelDown(CCharacter* pChr)
{
	m_pCharacter = pChr;
	if (m_pGameController->IsCroyaWarmup() || !m_pGameController->m_InfectedStarted) {
		TurnIntoNextHumanClass();
		return;
	}
	if (IsZombie() && IsInsideInfectionZone()) {
		TurnIntoNextZombieClass();
		return;
	}
	if (IsZombie()) {
		if (m_pCharacter && m_RespawnPointCooldown == 0 && !pChr->m_TaxiPassenger) {
			if (m_RespawnPointPlaced) {
				m_RespawnPointPos = m_pCharacter->GetPos();
			}
			else {
				m_RespawnPointPos = m_pCharacter->GetPos();
				m_RespawnPointPlaced = true;
			}
			m_RespawnPointCooldown = m_RespawnPointDefaultCooldown;
		}
		return;
	}
	m_pClass->OnMouseWheelDown(pChr);
}

void CroyaPlayer::OnMouseWheelUp(CCharacter* pChr)
{
	m_pCharacter = pChr;
	if (m_pGameController->IsCroyaWarmup() || !m_pGameController->m_InfectedStarted) {
		TurnIntoPrevHumanClass();
		return;
	}

	if (IsZombie() && IsInsideInfectionZone()) {
		TurnIntoPrevZombieClass();
		return;
	}
	m_pClass->OnMouseWheelUp(pChr);
}

void CroyaPlayer::ResetRespawnPoint()
{
	m_RespawnPointsNum = 1;
	m_RespawnPointPos = vec2(0, 0);
	m_RespawnPointPlaced = false;
}


vec2 CroyaPlayer::GetRespawnPointPos() const
{
	return m_RespawnPointPos;
}

int CroyaPlayer::GetRespawnPointsNum() const
{
	return m_RespawnPointsNum;
}

void CroyaPlayer::SetRespawnPointsNum(int Num)
{
	m_RespawnPointsNum = Num;
}

bool CroyaPlayer::IsRespawnPointPlaced() const
{
	return m_RespawnPointPlaced;
}

void CroyaPlayer::SetRespawnPointPlaced(bool Placed)
{
	m_RespawnPointPlaced = Placed;
}

int CroyaPlayer::GetRespawnPointDefaultCooldown() const
{
	return m_RespawnPointDefaultCooldown;
}

int CroyaPlayer::GetRespawnPointCooldown()
{
	return m_RespawnPointCooldown;
}

void CroyaPlayer::SetRespawnPointCooldown(int Cooldown)
{
	m_RespawnPointCooldown = Cooldown;
}

bool CroyaPlayer::IsHuman() const
{
	return !m_Infected;
}

bool CroyaPlayer::IsZombie() const
{
	return m_Infected;
}

bool CroyaPlayer::IsInitialZombie()
{
	return m_InitialZombie;
}


std::unordered_map<int, class IClass*>& CroyaPlayer::GetClasses()
{
	return m_Classes;
}

void CroyaPlayer::TurnIntoNextHumanClass()
{
	int NextClass = GetClassNum() + 1;
	int FirstClass = Class::HUMAN_CLASS_START + 1;
	bool NotInRange = !(NextClass > HUMAN_CLASS_START && NextClass < HUMAN_CLASS_END);

	if (NextClass == Class::HUMAN_CLASS_END || NotInRange)
		NextClass = FirstClass;
	SetClassNum(NextClass, false, true);
}

void CroyaPlayer::TurnIntoPrevHumanClass()
{
	int PrevClass = GetClassNum() - 1;
	int LastClass = Class::HUMAN_CLASS_END - 1;
	bool NotInRange = !(PrevClass > HUMAN_CLASS_START && PrevClass < HUMAN_CLASS_END);

	if (PrevClass == Class::HUMAN_CLASS_START || NotInRange)
		PrevClass = LastClass;
	SetClassNum(PrevClass, false, true);
}

//void CroyaPlayer::TurnIntoInitialZombie()
//{
//	m_InitialZombie = true;
//	TurnIntoRandomZombie();
//}

void CroyaPlayer::TurnIntoRandomZombie()
{
	int RandomZombieClass = random_int_range(Class::ZOMBIE_CLASS_START + 1, Class::WORKER - 1);
	SetClassNum(RandomZombieClass, true);

	if (m_pGameController->GetRealPlayerNum() >= 2 && m_pGameController->GetIZombieCount() < 1)
	{
		m_InitialZombie = true;
		m_pGameServer->SendBroadcast("You can choose ANY zombie class!", m_pPlayer->GetCID());
	}
}

void CroyaPlayer::TurnIntoRandomHuman()
{
	// Class::HUMAN_CLASS_START + 2 because of DEFAULT. DEFAULT comes right after HUMAN_CLASS_START
	int RandomHumanClass = random_int_range(Class::HUMAN_CLASS_START + 2, Class::HUMAN_CLASS_END - 1);
	SetClassNum(RandomHumanClass, true);
	ResetRespawnPoint();
}

void CroyaPlayer::TurnIntoNextZombieClass()
{
	int NextClass = GetClassNum() + 1;
	int FirstClass = Class::ZOMBIE_CLASS_START + 1;
	//int LastSelectableInGeneral = m_pGameServer->IsDevServer() ? PARASITE : MOTHER;
	int LastSelectableInGeneral = MOTHER;
	int SelectableZombieEnd = m_InitialZombie ? LastSelectableInGeneral + 1: MOTHER;
	bool NotInRange = !(NextClass > ZOMBIE_CLASS_START && NextClass < SelectableZombieEnd);

	if (NextClass == Class::ZOMBIE_CLASS_END || NotInRange)
		NextClass = FirstClass;
	SetClassNum(NextClass, false, true);
	ResetRespawnPoint();
}

void CroyaPlayer::TurnIntoPrevZombieClass()
{
	int PrevClass = GetClassNum() - 1;
	//int LastSelectableInGeneral = m_pGameServer->IsDevServer() ? PARASITE : MOTHER;
	//TBD
	int LastSelectableInGeneral = MOTHER;
	int SelectableZombieEnd = m_InitialZombie ? LastSelectableInGeneral + 1: MOTHER;
	bool NotInRange = !(PrevClass > ZOMBIE_CLASS_START && PrevClass < SelectableZombieEnd);

	if (PrevClass == Class::ZOMBIE_CLASS_START || NotInRange)
		PrevClass = SelectableZombieEnd - 1;
	SetClassNum(PrevClass, false, true);
	ResetRespawnPoint();
}

bool CroyaPlayer::IsInsideInfectionZone() const
{
	return m_InsideInfectionZone;
}

void CroyaPlayer::SetInsideInfectionZone(bool InsideInfectionZone)
{
	m_InsideInfectionZone = InsideInfectionZone;
}

void CroyaPlayer::SetInsideSafeZone(bool InsideSafeZone)
{
	m_InsideSafeZone = InsideSafeZone;
}

int CroyaPlayer::GetAirJumpCounter() const
{
	return m_AirJumpCounter;
}

void CroyaPlayer::SetAirJumpCounter(int AirJumpCounter)
{
	m_AirJumpCounter = AirJumpCounter;
}

vec2 CroyaPlayer::GetLockPosition() {
	return m_PositionLockVec;
}

bool CroyaPlayer::IsPositionLocked() {
	return m_IsPositionLocked;
}

bool CroyaPlayer::IsAntigravityOn() {
	return m_IsAntigravityOn;
}

void CroyaPlayer::ResetCanLockPositionAbility() {
	m_CanLockPosition = true;
}

void CroyaPlayer::AntigravityOn(bool LooseSpeed, int Duration) {
	dbg_msg("game", "Antigravity on");
	m_IsAntigravityOn = true;
	GetClass()->AntigravityOn(GetCharacter(), LooseSpeed, Duration);
}

void CroyaPlayer::AntigravityOff() {
	dbg_msg("game", "Antigravity off");
	SetAirJumpCounter(10);
	m_IsAntigravityOn = false;
}

void CroyaPlayer::LockPosition(vec2 Pos)
{
	if (m_pGameController->IsCroyaWarmup())
	  return;

	if (m_IsPositionLocked || !m_CanLockPosition)
		return;
	m_IsPositionLocked = true;
	m_CanLockPosition = false;
	m_PositionLockVec = Pos;
}

void CroyaPlayer::UnlockPosition()
{
	m_IsPositionLocked = false;
}

bool CroyaPlayer::IsHookProtected() const
{
	return m_HookProtected;
}

void CroyaPlayer::SetHookProtected(bool HookProtected)
{
	if (HookProtected && m_pCharacter && (m_pCharacter->IsTaxiPassenger() || m_pCharacter->IsTaxiDriver() || m_pCharacter->m_FreeTaxi))
		m_pCharacter->ResetTaxi();
	m_HookProtected = HookProtected;
	if (m_pPlayer) {
		m_pPlayer->SetHookProtected(HookProtected);
		if (IsHookProtected()) {
			m_pGameServer->SendChatTarget(m_ClientID, "Hook protection enabled");
		} else {
			m_pGameServer->SendChatTarget(m_ClientID, "Hook protection disabled");

		}
	}
}

int CroyaPlayer::GetOldClassNum() const
{
	return m_OldClassNum;
}

void CroyaPlayer::SetOldClassNum(int Class)
{
	if (Class == Class::DEFAULT) {
		Class = Class::MEDIC;
	}
	m_OldClassNum = Class;
}

const char* CroyaPlayer::GetLanguage() const
{
	return m_Language.c_str();
}

void CroyaPlayer::SetLanguage(const char* Language)
{
	m_Language = Language;
}

CGameControllerMOD* CroyaPlayer::GetGameControllerMOD()
{
	return m_pGameController;
}

IClass* CroyaPlayer::GetClass()
{
	return m_pClass;
}

void CroyaPlayer::SetClass(IClass* pClass, bool DrawPurpleThing, bool destroyChildEntities)
{
	if (m_pCharacter && DrawPurpleThing) {
		vec2 PrevPos = m_pCharacter->GetPos();
		m_pGameServer->CreatePlayerSpawn(PrevPos); // draw purple thing
	}

	if (!pClass)
		return;
	m_pClass = pClass;
	const CSkin& skin = m_pClass->GetSkin();
    //m_pPlayer->m_TeeInfos.m_UseCustomColor = 1;
    //m_pPlayer->m_TeeInfos.m_ColorBody = skin.GetBodyColor();
    //m_pPlayer->m_TeeInfos.m_ColorFeet = skin.GetFeetColor(); 
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[0] = true;
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[1] = true;
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[2] = true;
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[3] = true;
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[4] = true;
	m_pPlayer->m_TeeInfos.m_aUseCustomColors[5] = true;
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[0] = skin.GetBodyColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[1] = skin.GetMarkingColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[2] = skin.GetDecorationColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[3] = skin.GetHandsColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[4] = skin.GetFeetColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[5] = skin.GetEyesColor();
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[0], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[0]), "%s", skin.GetBodyName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[1], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[1]), "%s", skin.GetMarkingName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[2], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[2]), "%s", skin.GetDecorationName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[3], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[3]), "%s", skin.GetHandsName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[4], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[4]), "%s", skin.GetFeetName());
	if (m_pGameServer->Server()->IsSixup(m_ClientID))
		str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[5], sizeof(FCLIENT_STRING), "%s", FCLIENT_STRING);
	else
		str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[5], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[5]), "%s", skin.GetEyesName());

	if (m_pClass->m_06SkinName[0] != '\0') {
		dbg_msg("Setting skin %s", m_pClass->m_06SkinName);
		m_pPlayer->m_TeeInfos.Set06Skin(m_pClass->m_06SkinName);
		if (m_pClass->m_06SkinBodyColor) {
			m_pPlayer->m_TeeInfos.m_UseCustomColor = 1;
			m_pPlayer->m_TeeInfos.m_ColorBody = m_pClass->m_06SkinBodyColor;
			m_pPlayer->m_TeeInfos.m_ColorFeet = m_pClass->m_06SkinFeetColor;
			dbg_msg("Setting skin custom colors", m_pClass->m_06SkinName);
		} else {
			m_pPlayer->m_TeeInfos.m_UseCustomColor = 0;
		}
	}
	else
		m_pPlayer->m_TeeInfos.FromSixup();

	if (m_pClass->IsInfectedClass()) {
		m_Infected = true;
		if (GetCharacter())
			GetCharacter()->SetInfected(true);
	}
	else {
		m_InitialZombie = false;
		m_Infected = false;
		if (GetCharacter())
			GetCharacter()->SetInfected(false);
	}


	std::string NewClan = "Unknown";
	if (m_pClass->IsInfectedClass()) {
		if (m_InitialZombie) {
			NewClan = std::string("i") + m_pClass->m_Name;
		}
		else {
			NewClan = m_pClass->m_Name;
		}
	} else {
		NewClan = m_pClass->m_Name;
	}

	m_pGameServer->Server()->SetClientClan(m_ClientID, NewClan.c_str());

 	for (const CPlayer* each : m_pGameServer->m_apPlayers) {
		if (each) {
			m_pGameServer->SendSkinChange(m_pPlayer->GetCID(), each->GetCID());
			m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), NewClan.c_str());
		}
	}
/* 	for (const CPlayer* each : m_pGameServer->m_apPlayers) {
		if (each) {
			m_pGameServer->SendSkinChange(m_pPlayer->GetCID(), each->GetCID());
			if (m_pClass->IsInfectedClass()) {
				if (m_InitialZombie)
				  m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "iZombie");
				else
				  m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "Zombie");
			} else {
				m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "Human");
			}
		}
	} */
	if (destroyChildEntities) {
		if (m_pCharacter) {
			m_pCharacter->DestroyChildEntities();
		}
	}

	if (m_pCharacter) {
		m_pCharacter->SetInfected(m_pClass->IsInfectedClass()); // double call?
		m_pCharacter->m_EndlessHook = false;
		m_pCharacter->ResetWeaponsHealth();
		m_pClass->InitialWeaponsHealth(m_pCharacter);
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s", m_pClass->GetName().c_str());
	//m_pGameServer->SendBroadcast(localize(aBuf, GetLanguage()).c_str(), m_pPlayer->GetCID());
	m_pGameServer->SendBroadcast(aBuf, m_pPlayer->GetCID());
	m_IsAntigravityOn = false;
}
