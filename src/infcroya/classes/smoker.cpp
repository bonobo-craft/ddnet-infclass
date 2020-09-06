#include "smoker.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

CSmoker::CSmoker() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(80, 140, 70);
	skin.SetMarkingName("cammo2");
	skin.SetMarkingColor(75, 0, 0, 190);
	skin.SetHandsColor(75, 135, 45);
	skin.SetFeetColor(80, 50, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Smoker");
	Set06SkinName("cammo");
	Set06SkinColors(5278720, 5255680);
}

void CSmoker::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CSmoker::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;
	}
}
