#include "bat.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <game/generated/protocol7.h>

CBat::CBat() : IClass()
{
	CSkin skin;
	skin.SetBodyName("bat");
	skin.SetBodyColor(168, 65, 31);
	skin.SetMarkingName("bear");
	skin.SetMarkingColor(190, 255, 0, 134);
	skin.SetHandsColor(167, 76, 0);
	skin.SetFeetColor(185, 187, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Bat");
}

void CBat::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(8);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CBat::Tick(CCharacter* pChr)
{
	//Double jumps
	CroyaPlayer* cp = pChr->GetCroyaPlayer();
	if (pChr->IsGrounded()) cp->SetAirJumpCounter(0);
	if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP && cp->GetAirJumpCounter() < 1)
	{
		pChr->GetCharacterCore().m_Jumped &= ~2;
		cp->SetAirJumpCounter(0);
	}
}

void CBat::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;
	}
}