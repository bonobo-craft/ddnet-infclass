#include "worker.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>

CWorker::CWorker() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(75, 150, 25);
	skin.SetMarkingName("whisker");
	skin.SetMarkingColor(80, 255, 100, 255);
	skin.SetDecorationName("hair");
	skin.SetHandsColor(75, 150, 25);
	skin.SetFeetColor(75, 100, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Worker");
}

void CWorker::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(8);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CWorker::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;
	}
}