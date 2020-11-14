#include "magician.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/entities/projectile.h>
#include <game/server/gamecontext.h>
#include <game/generated/server_data.h>
#include <game/server/entities/laser.h>
#include <infcroya/croyaplayer.h>
#include <infcroya/entities/scatter-grenade.h>

CMagician::CMagician()
{
	CSkin skin;
	skin.SetBodyName("standard");
	skin.SetMarkingName("tiger2");
	skin.SetDecorationName("unipento");
	//skin.SetBodyColor(189, 255, 145);
	skin.SetBodyColor(217, 227, 51);
	skin.SetMarkingColor(206, 152, 191, 201);
	skin.SetDecorationColor(0, 255, 156);
	skin.SetHandsColor(208, 107, 136);
	skin.SetFeetColor(220, 166, 150);
	//skin.SetFeetColor(209, 158, 24);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Magician");
	LockPositionTimeLeft = 0;
	Set06SkinName("redbopp");
}

void CMagician::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_LASER, 10);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->GiveWeapon(WEAPON_GRENADE, 10);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

void CMagician::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		//LockOrUnlockPosition(pChr);
		HammerShoot(pChr, ProjStartPos);
		pGameServer->CreateDeath(pChr->m_Pos, pChr->GetPlayer()->GetCID());
	} break;

	case WEAPON_GUN: {
		GunShoot(pChr, ProjStartPos, Direction);
	} break;

	case WEAPON_LASER: {
		//new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		// if (pChr->GetCroyaPlayer()->IsPositionLocked())
		// 	new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach, pChr->GetPlayer()->GetCID());
		pGameServer->CreateSound(pChr->GetPos(), SOUND_LASER_FIRE);
	} break;

	case WEAPON_GRENADE: {
		//Find bomb
		bool BombFound = false;
		for (CScatterGrenade* pGrenade = (CScatterGrenade*)pGameWorld->FindFirst(CGameWorld::ENTTYPE_SCATTER_GRENADE); pGrenade; pGrenade = (CScatterGrenade*)pGrenade->TypeNext())
		{
			if (pGrenade->m_Owner != ClientID) continue;
			pGrenade->Explode();
			BombFound = true;
		}

		if (!BombFound && pChr->m_aWeapons[WEAPON_GRENADE].m_Ammo)
		{
			int ShotSpread = 2;

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread * 2 + 1);

			for (int i = -ShotSpread; i <= ShotSpread; ++i)
			{
				float a = angle(Direction) + frandom() / 3.0f;

				CScatterGrenade* pProj = new CScatterGrenade(pGameWorld, ClientID, pChr->GetPos(), vec2(cosf(a), sinf(a)));

				// pack the Projectile and send it to the client Directly
				CNetObj_Projectile p;
				pProj->FillInfo(&p);

				for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
					Msg.AddInt(((int*)& p)[i]);
				pChr->Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
			}

			pGameServer->CreateSound(pChr->GetPos(), SOUND_GRENADE_FIRE);

			pChr->SetReloadTimer(pChr->Server()->TickSpeed() / 4);
			pChr->m_aWeapons[WEAPON_GRENADE].m_Ammo++;
		}
	} break;
	}
}

void CMagician::LockOrUnlockPosition(CCharacter* pChr) {
	if (!pChr->GetCroyaPlayer())
		return;

	if (pChr->GetCroyaPlayer()->IsPositionLocked()) {
		UnlockPosition(pChr);
	} else {
		LockPosition(pChr);
	}
}

void CMagician::GunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction_) {
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();
	CGameWorld* pGameWorld = pChr->GameWorld();

	new CProjectile(pGameWorld, WEAPON_GUN,
		ClientID,
		ProjStartPos,
		Direction_,
		(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GunLifetime),
		g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 10, -1, WEAPON_GUN);

	pGameServer->CreateSound(pChr->GetPos(), SOUND_GUN_FIRE);
}

void CMagician::Tick(CCharacter* pChr)
{
 	//if (pChr->GetCharacterCore().m_PressedJump)
	//  UnlockPosition(pChr);

	ItAntigravitates(pChr);
	ItSelfAntigravitates(pChr);
	//ItDoubleJumps(pChr);
	//ItLocksInSpace(pChr);
}

void CMagician::LockPosition(CCharacter* pChr) {
	if (pChr->IsGrounded())
	  return;
	
	pChr->GetCroyaPlayer()->LockPosition(pChr->GetCharacterCore().m_Pos);
	LockPositionTimeLeft = pChr->Server()->TickSpeed() * 5;
	//g_Config.m_InfMagicianPositionLockTime;
}

void CMagician::UnlockPosition(CCharacter* pChr) {
	if (!pChr->GetCroyaPlayer())
		return;
	pChr->GetCroyaPlayer()->UnlockPosition();	
}

void CMagician::ItLocksInSpace(CCharacter* pChr) {
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