#include "biologist.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/entities/biologist-mine.h>
#include <infcroya/entities/poison-circle.h>
#include <generated/server_data.h>
#include <game/server/entities/projectile.h>
#include <infcroya/entities/bouncing-bullet.h>

CBiologist::CBiologist()
{
	CSkin skin;
	skin.SetBodyColor(52, 156, 124);
	skin.SetMarkingName("twintri");
	skin.SetMarkingColor(40, 222, 227);
	skin.SetFeetColor(147, 4, 72);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Biologist");
}

void CBiologist::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->GiveWeapon(WEAPON_SHOTGUN, 10);
	pChr->GiveWeapon(WEAPON_LASER, 10);
	pChr->SetWeapon(WEAPON_GUN);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

bool CBiologist::WillItFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr) {
	if (Weapon == WEAPON_LASER) {
		if (pChr->m_aWeapons[WEAPON_LASER].m_Ammo < 10)
		  return false;		
	}
	return true;
}

void CBiologist::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
		RemovePoisonNearby(pChr);
	} break;

	case WEAPON_GUN: {
		new CProjectile(pGameWorld, WEAPON_GUN,
			ClientID,
			ProjStartPos,
			Direction,
			(int)(pChr->Server()->TickSpeed() * pChr->GameServer()->Tuning()->m_GunLifetime),
			g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 0, -1, WEAPON_GUN);

		pGameServer->CreateSound(pChr->GetPos(), SOUND_GUN_FIRE);
	} break;

	case WEAPON_SHOTGUN: {
		int ShotSpread = 2;
		CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
		Msg.AddInt(ShotSpread * 2 + 1);

		for (int i = -ShotSpread; i <= ShotSpread; ++i)
		{
			float Spreading[] = { -0.185f, -0.070f, 0, 0.070f, 0.185f };
			float a = angle(Direction);
			a += Spreading[i + 2];
			float v = 1 - (absolute(i) / (float)ShotSpread);
			float Speed = mix((float)pChr->GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
			
			CBouncingBullet* pProj = new CBouncingBullet(pChr->GameWorld(), pChr->GetPlayer()->GetCID(), ProjStartPos, vec2(cosf(a), sinf(a)) * Speed);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);
			for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
				Msg.AddInt(((int*)& p)[i]);
		}

		pChr->GameServer()->CreateSound(pChr->GetPos(), SOUND_SHOTGUN_FIRE);
	} break;

	case WEAPON_LASER: {
		CGameWorld* pGameWorld = pChr->GameWorld();
		if (pChr->m_aWeapons[WEAPON_LASER].m_Ammo < 10) {
			pChr->SetReloadTimer(125 * pChr->Server()->TickSpeed() / 1000);
			if (pChr->GetLastNoAmmoSound() + pChr->Server()->TickSpeed() <= pChr->Server()->Tick())
			{
				pChr->GameServer()->CreateSound(pChr->GetPos(), SOUND_WEAPON_NOAMMO);
				pChr->SetLastNoAmmoSound(pChr->Server()->Tick());
			}
			return;
		}
		vec2 To = pChr->GetPos() + Direction * 400.0f;
		if (pChr->GameServer()->Collision()->IntersectLine(pChr->GetPos(), To, 0x0, &To))
		{
			for (CBiologistMine* pMine = (CBiologistMine*)pGameWorld->FindFirst(CGameWorld::ENTTYPE_BIOLOGIST_MINE); pMine; pMine = (CBiologistMine*)pMine->TypeNext())
			{
				if (pMine->m_Owner != pChr->GetPlayer()->GetCID()) continue;
				pChr->GameServer()->m_World.DestroyEntity(pMine);
			}
			new CBiologistMine(pGameWorld, pChr->GetPos(), To, pChr->GetPlayer()->GetCID());
			pChr->GameServer()->CreateSound(pChr->GetPos(), SOUND_LASER_FIRE);
			pChr->m_aWeapons[WEAPON_LASER].m_Ammo = 0;
		}
	} break;
	}
}

void CBiologist::RemovePoisonNearby(CCharacter* pChr)
{
	//int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

    CPoisonCircle *pPoisonCircle = (CPoisonCircle *)pGameWorld->FindFirst(CGameWorld::ENTTYPE_POISON_CIRCLE);
    
    while(pPoisonCircle) {
   		
		float Len = distance(pChr->GetPos(), pPoisonCircle->GetPos());
		
		if(Len < pChr->GetProximityRadius()+g_Config.m_InfPoisonCircleRadius){
			pGameServer->m_World.DestroyEntity(pPoisonCircle);
		}

        pPoisonCircle = (CPoisonCircle*)pPoisonCircle->TypeNext();
    }
}