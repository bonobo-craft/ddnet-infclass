#include "freezer.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <infcroya/entities/poison-circle.h>

CFreezer::CFreezer() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(98, 200, 120);
	skin.SetMarkingName("mixture2");
	skin.SetMarkingColor(90, 255, 125, 255);
	skin.SetHandsColor(90, 125, 50);
	skin.SetFeetColor(120, 160, 70);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Cryo");
	Set06SkinName("jeet");
	Set06SkinColors(6473777, 7905280);
}

void CFreezer::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CFreezer::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);			
	} break;
	}
}

int CFreezer::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
    return 0;
}

