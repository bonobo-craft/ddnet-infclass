#include "sniper.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/entities/projectile.h>
#include <game/server/gamecontext.h>
#include <generated/server_data.h>
#include <game/server/entities/laser.h>
#include <infcroya/croyaplayer.h>

CSniper::CSniper()
{
	CSkin skin;
	skin.SetBodyName("standard");
	skin.SetBodyColor(29, 173, 87);
	skin.SetMarkingName("warpaint");
	skin.SetMarkingColor(0, 0, 255);
	skin.SetHandsColor(11, 115, 1);
	skin.SetFeetColor(29, 173, 87);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Sniper");
	LockPositionTimeLeft = 0;
}

void CSniper::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_LASER, 10);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

void CSniper::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		LockOrUnlockPosition(pChr);
		HammerShoot(pChr, ProjStartPos);
	} break;

	case WEAPON_GUN: {
		new CProjectile(pGameWorld, WEAPON_GUN,
			ClientID,
			ProjStartPos,
			Direction,
			(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GunLifetime),
			g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 0, -1, WEAPON_GUN);

		pGameServer->CreateSound(pChr->GetPos(), SOUND_GUN_FIRE);
	} break;

	case WEAPON_LASER: {
		new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		if (pChr->GetCroyaPlayer()->IsPositionLocked())
			new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		pGameServer->CreateSound(pChr->GetPos(), SOUND_LASER_FIRE);
	} break;
	}
}

void CSniper::LockOrUnlockPosition(CCharacter* pChr) {
	if (!pChr->GetCroyaPlayer())
		return;

	if (pChr->GetCroyaPlayer()->IsPositionLocked()) {
		UnlockPosition(pChr);
	} else {
		LockPosition(pChr);
	}
}

void CSniper::Tick(CCharacter* pChr)
{
	if (pChr->GetCharacterCore().m_PressedJump)
	  UnlockPosition(pChr);

	ItDoubleJumps(pChr);
	ItLocksInSpace(pChr);
}

void CSniper::LockPosition(CCharacter* pChr) {
	if (pChr->IsGrounded())
	  return;
	
	pChr->GetCroyaPlayer()->LockPosition(pChr->GetCharacterCore().m_Pos);
	LockPositionTimeLeft = pChr->Server()->TickSpeed() * g_Config.m_InfSniperPositionLockTime;
}

void CSniper::UnlockPosition(CCharacter* pChr) {
	pChr->GetCroyaPlayer()->UnlockPosition();	
}

void CSniper::ItLocksInSpace(CCharacter* pChr) {
	CroyaPlayer* cp = pChr->GetCroyaPlayer();

	if (pChr->IsGrounded())
		cp->ResetCanLockPositionAbility();

	if (cp->IsPositionLocked()) {
		pChr->GetCharacterCore().m_Pos = cp->GetLockPosition();
		pChr->GetCharacterCore().m_Vel = vec2(0.f, -1.5f); // gravity prediction compensation
		LockPositionTimeLeft--;
	
		if(LockPositionTimeLeft < 0)
		{
			UnlockPosition(pChr);
		}
	}	
}