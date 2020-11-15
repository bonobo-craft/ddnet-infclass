#include "spider.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <game/server/entities/projectile.h>

CSpider::CSpider() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(87, 227, 51);
	skin.SetMarkingName("wildpatch");
	skin.SetDecorationName("twinpen");
	skin.SetMarkingColor(206, 0, 50, 251);
	//skin.SetMarkingColor(43, 205, 98, 255); // yellow
	skin.SetDecorationColor(15, 176, 60);
	skin.SetHandsColor(208, 0, 57);
	skin.SetFeetColor(220, 0, 55);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Spider");
	Set06SkinName("teerasta");
	Set06SkinColors(5152256, 3608832);
}

void CSpider::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	//pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
	pChr->m_EndlessHook = true;
}

void CSpider::Tick(CCharacter* pChr)
{
	ItFastJumps(pChr);
	ItAntigravitates(pChr);
	pChr->m_EndlessHook = true;
}

void CSpider::ItFastJumps(CCharacter* pChr) {
	//Double jumps
	CroyaPlayer* cp = pChr->GetCroyaPlayer();
	if (pChr->IsGrounded()) {
		cp->SetAirJumpCounter(0);
		return;
	}
	if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP && cp->GetAirJumpCounter() == 0)
	{
		pChr->GetCharacterCore().m_Jumped &= ~2;
		pChr->GetCharacterCore().m_Vel = vec2(pChr->GetCharacterCore().m_Vel.x * 2,
		pChr->GetCharacterCore().m_Vel.y);
		cp->SetAirJumpCounter(cp->GetAirJumpCounter() + 1);
		return;
	}
	// if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP && cp->GetAirJumpCounter() == 1)
	// {
	// 	pChr->GetCharacterCore().m_Vel = vec2(pChr->GetCharacterCore().m_Vel.x * -1.0f,
	// 	pChr->GetCharacterCore().m_Vel.y);
	// 	cp->SetAirJumpCounter(cp->GetAirJumpCounter() + 1);
	// 	return;
	// }
}


void CSpider::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	}
	break;
	}
}

