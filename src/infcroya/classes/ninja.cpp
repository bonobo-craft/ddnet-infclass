#include "ninja.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/projectile.h>

CNinja::CNinja()
{
	CSkin skin;
	skin.SetBodyColor(155, 0, 122);
	skin.SetMarkingName("default");
	skin.SetMarkingColor(0, 0, 255);
	skin.SetFeetColor(29, 0, 87);
	SetSkin(skin);
	SetInfectedClass(false);
	SetName("Ninja");
	Set06SkinName("coala");
}

void CNinja::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	//pChr->GiveWeapon(WEAPON_NINJA, -1);
	pChr->SetNormalEmote(EMOTE_NORMAL);
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
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;
	}
}
