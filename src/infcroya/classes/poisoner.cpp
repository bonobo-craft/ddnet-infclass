#include "poisoner.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <infcroya/entities/poison-circle.h>

CPoisoner::CPoisoner() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(60, 255, 30);
	skin.SetMarkingName("wildpatch");
	skin.SetMarkingColor(43, 255, 88, 120);
	skin.SetHandsColor(60, 150, 90);
	skin.SetFeetColor(60, 125, 50);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Poisoner");
	Set06SkinName("antiantey");
	Set06SkinColors(3997440, 3964160);
}

void CPoisoner::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CPoisoner::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();

	switch (Weapon) {
	case WEAPON_HAMMER: {
			//PlacePoison(pChr);
			pChr->Die(ClientID, WEAPON_SELF);
	} break;
	}
}

int CPoisoner::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	PlacePoison(pVictim);
    return 0;
}

void CPoisoner::PlacePoison(CCharacter* pChr)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameWorld* pGameWorld = pChr->GameWorld();
	CGameContext* pGameServer = pChr->GameServer();

    CPoisonCircle *p = (CPoisonCircle *)pGameWorld->FindFirst(CGameWorld::ENTTYPE_POISON_CIRCLE);
    
    while(p) {
        if (p->GetOwner() == ClientID) {
    		pGameServer->m_World.DestroyEntity(p);
        }
        p = (CPoisonCircle*)p->TypeNext();
    }

	new CPoisonCircle(pGameWorld, pChr->GetPos(), ClientID);
}