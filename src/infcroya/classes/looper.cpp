#include "looper.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/entities/projectile.h>
#include <game/server/gamecontext.h>
#include <game/generated/server_data.h>
#include <game/server/entities/laser.h>
#include <infcroya/entities/looper-wall.h>

CLooper::CLooper()
{
	CSkin skin;
	skin.SetBodyName("kitty");
	skin.SetBodyColor(132, 118, 184);
	skin.SetMarkingName("whisker");
	skin.SetMarkingColor(130, 109, 219);
	skin.SetHandsColor(120, 82, 232);
	skin.SetFeetColor(120, 82, 232);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Looper");
	Set06SkinName("bluekitty");
}

void CLooper::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->GiveWeapon(WEAPON_LASER, 10);
	pChr->SetWeapon(WEAPON_GUN);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

void CLooper::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		for (CLooperWall* pWall = (CLooperWall*)pGameWorld->FindFirst(CGameWorld::ENTTYPE_LOOPER_WALL); pWall; pWall = (CLooperWall*)pWall->TypeNext())
		{
			if (pWall->m_Owner == ClientID)
				pGameServer->m_World.DestroyEntity(pWall);
		}

		if (pChr->m_FirstShot)
		{
			pChr->m_FirstShot = false;
			pChr->m_FirstShotCoord = pChr->GetPos();
		}
		else if (distance(pChr->m_FirstShotCoord, pChr->GetPos()) > 10.0)
		{
			/*//Check if the barrier is in toxic gases
			bool isAccepted = true;
			for (int i = 0; i < 15; i++)
			{
				vec2 TestPos = m_FirstShotCoord + (m_Pos - m_FirstShotCoord) * (static_cast<float>(i) / 14.0f);
				if (pGameServer->Collision()->GetZoneValueAt(pGameServer->m_ZoneHandle_Damage, TestPos) == ZONE_DAMAGE_INFECTION)
				{
					isAccepted = false;
				}
			}*/
			bool isAccepted = true;
			if (isAccepted)
			{
				pChr->m_FirstShot = true;

				new CLooperWall(pGameWorld, pChr->m_FirstShotCoord, pChr->GetPos(), ClientID);

				pGameServer->CreateSound(pChr->GetPos(), SOUND_LASER_FIRE);
			}
		}
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
		new CLaser(pGameWorld, pChr->GetPos(), Direction, pGameServer->Tuning()->m_LaserReach * 0.6f, pChr->GetPlayer()->GetCID());
		pGameServer->CreateSound(pChr->GetPos(), SOUND_LASER_FIRE);
	} break;
	}
}
