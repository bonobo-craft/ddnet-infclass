#include "hunter.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <game/server/entities/projectile.h>

CHunter::CHunter() : IClass()
{
	CSkin skin;
	skin.SetBodyColor(65, 255, 0);
	skin.SetMarkingName("setisu");
	skin.SetMarkingColor(59, 255, 22, 255);
	skin.SetHandsColor(65, 255, 20);
	skin.SetFeetColor(100, 255, 0);
	SetSkin(skin);
	SetInfectedClass(true);
	SetName("Hunter");
}

void CHunter::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_ANGRY);
}

void CHunter::Tick(CCharacter* pChr)
{
	ItDoubleJumps(pChr);
}

void CHunter::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
	case WEAPON_HAMMER: {
		HammerShoot(pChr, ProjStartPos);
	} break;

	case WEAPON_GUN:
	{
		int ClientID = pChr->GetPlayer()->GetCID();
		CGameContext *pGameServer = pChr->GameServer();
		CGameWorld *pGameWorld = pChr->GameWorld();

		CCharacter *apCloseCCharacters[MAX_CLIENTS];
		int Num = pGameServer->m_World.FindEntities(pChr->GetPos(), 9999, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for(int i = 0; i < Num; i++)
			{
				CCharacter *human = apCloseCCharacters[i];
				if (!human->IsAlive() || human->GetPlayer()->GetTeam() == TEAM_SPECTATORS)
					continue;					

				if (human->IsZombie())
				  continue;

				vec2 Direction = normalize(vec2(human->GetPos().x - pChr->GetPos().x, human->GetPos().y - pChr->GetPos().y));

				new CProjectile(pGameWorld, WEAPON_GUN,
						ClientID,
						ProjStartPos,
						Direction,
						(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GunLifetime),
						0, false, 0, -1, WEAPON_GUN);
				return;
			}
	}
	break;
	}
}

