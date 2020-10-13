#include "ninja.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/projectile.h>

CNinja::CNinja()
{
	CSkin skin;
	skin.SetBodyColor(0, 0, 0);
	skin.SetMarkingName("uppy");
	skin.SetMarkingColor(0, 0, 255, 75);
	skin.SetDecorationName("twinmello");
	skin.SetDecorationColor(0, 0, 0);
	skin.SetFeetColor(0, 0, 255);
	skin.SetHandsColor(0, 0, 0);
	skin.SetEyesName("negative");
	skin.SetEyesColor(102, 0, 255);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Ninja");
	Set06SkinName("coala");
}

void CNinja::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, -1);
	//pChr->GiveWeapon(WEAPON_NINJA, -1);
	pChr->SetNormalEmote(EMOTE_NORMAL);
	pChr->SetWeapon(WEAPON_HAMMER);
}

void CNinja::OnMouseWheelDown(CCharacter* pChr) {
	if (pChr)
	  pChr->GiveNinjaIfGrounded();
}

void CNinja::OnMouseWheelUp(CCharacter* pChr) {
	if (pChr)
	  pChr->RemoveNinja();
}

void CNinja::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
/* 	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer(); */

	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;
	}
}
