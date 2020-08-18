#include "parasite.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <infcroya/entities/poison-circle.h>

CParasite::CParasite() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(98, 0, 60);
	skin.SetMarkingName("mixture2");
	skin.SetMarkingColor(90, 0, 125, 255);
	skin.SetHandsColor(90, 125, 50);
	skin.SetFeetColor(120, 160, 70);
	skin.SetEyesName("negative");
	skin.SetEyesColor(1, 244, 104);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Parasite");
}

void CParasite::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CParasite::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);			
	} break;
	}
}

int CParasite::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
    return 0;
}

