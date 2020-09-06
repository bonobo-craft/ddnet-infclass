#include "hero.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/projectile.h>
#include <game/server/entities/flag.h>
#include <infcroya/entities/bouncing-bullet.h>

CHero::CHero()
{
	CSkin skin;
	skin.SetBodyColor(155, 166, 122);
	skin.SetMarkingName("twintri");
	skin.SetMarkingColor(155, 166, 122);
	skin.SetDecorationName("twinbopp");
	skin.SetDecorationColor(233, 158, 183);
	skin.SetHandsColor(0, 200, 200);
	skin.SetFeetColor(0, 200, 200);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Hero");
	Set06SkinName("redstripe");
}

void CHero::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GRENADE, 10);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->SetWeapon(WEAPON_GRENADE);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

void CHero::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;


	case WEAPON_GRENADE: {
		GrenadeShoot(pChr, ProjStartPos, Direction); {
	} break;
	}

	case WEAPON_GUN: {
		int ClientID = pChr->GetPlayer()->GetCID();
		CGameContext* pGameServer = pChr->GameServer();
		CGameWorld* pGameWorld = pChr->GameWorld();

    	CFlag *pFlag = (CFlag*)pGameServer->m_World.FindFirst(CGameWorld::ENTTYPE_FLAG);

		if (pFlag == 0)
		  break;

    	vec2 Direction = normalize(vec2(pFlag->GetPos().x-pChr->GetPos().x, pFlag->GetPos().y-pChr->GetPos().y));
		  
		new CProjectile(pGameWorld, WEAPON_GUN,
			ClientID,
			ProjStartPos,
			Direction,
			(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GunLifetime),
			0, false, 0, -1, WEAPON_GUN);

		pGameServer->CreateSound(pChr->GetPos(), SOUND_GUN_FIRE);
	} break;
	}

}
