#include "boomer.h"
#include "base/system.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

CBoomer::CBoomer() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(87, 255, 17);
	skin.SetMarkingName("saddo");
	skin.SetMarkingColor(0, 200, 0, 255);
	skin.SetDecorationName("unibop");
	skin.SetDecorationColor(78, 255, 0);
	skin.SetHandsColor(79, 255, 0);
	skin.SetFeetColor(81, 139, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Boomer");
	Set06SkinName("voodoo_tee");
	Set06SkinColors(5766912, 5344000);
}

void CBoomer::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CBoomer::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();

	switch (Weapon) {
		case WEAPON_HAMMER:
		{
			// he will explode in OnCharacterDeath
			//BoomerExplosion(pChr);
			pChr->Die(ClientID, WEAPON_SELF);
		}
	}
}

int CBoomer::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	if (Weapon != WEAPON_NINJA)
	  BoomerExplosion(pVictim);
	return 0;
}

void CBoomer::BoomerExplosion(CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();
	if (pChr->Stunned())
		return;
	//if( !IsFrozen() && !IsInLove() ) not implemented yet
	//{

		pGameServer->CreateSound(pChr->GetPos(), SOUND_GRENADE_EXPLODE);
		float force = g_Config.m_InfBoomerForce / 10.0f;
		float innerRadius = g_Config.m_InfBoomerInnerRadius / 10.0f;
		float damageRadius = g_Config.m_InfBoomerDamageRadius / 10.0f;
		float damage = g_Config.m_InfBoomerDamage / 10.0f;
		
		pGameServer->CreateExplosionDisk(pChr->GetPos(), innerRadius, damageRadius, damage, force, ClientID, WEAPON_HAMMER);
	//}
}
