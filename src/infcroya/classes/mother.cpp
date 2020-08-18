#include "mother.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>

CMother::CMother() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(75, 150, 25);
	skin.SetMarkingName("whisker");
	skin.SetMarkingColor(80, 255, 100, 255);
	skin.SetDecorationName("unipento");
	skin.SetDecorationColor(45, 255, 100);
	skin.SetHandsColor(75, 150, 25);
	skin.SetFeetColor(75, 100, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Queen");
}

void CMother::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CMother::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
			//MotherPlaceRespawn(pChr);
			//pChr->Die(ClientID, WEAPON_SELF);
	} break;
	}
}

void CMother::MotherPlaceRespawn(CCharacter* pChr)
{
    CGameContext* pGameServer = pChr->GameServer();

    for (CPlayer *each : pChr->GameServer()->m_apPlayers)
    {
        if (!each)
            continue;
        if (!each->GetCroyaPlayer())
            continue;

        if (each->GetCroyaPlayer()->IsHuman())
            continue;

        if (each == pChr->GetPlayer()) {
            continue;
        }

        each->GetCroyaPlayer()->SetSpawnPointAt(pChr->GetPos());
        
    }

   
	pGameServer->CreateSound(pChr->GetPos(), SOUND_CTF_RETURN);
}