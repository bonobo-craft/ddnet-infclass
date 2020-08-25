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
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();

	switch (Weapon) {
	case WEAPON_HAMMER: {
		// reset objects Hit
		pChr->SetNumObjectsHit(0);
		pGameServer->CreateSound(pChr->GetPos(), SOUND_HAMMER_FIRE);

		//find other chars
		CCharacter* apEnts[MAX_CLIENTS];
		int Hits = 0;
		int Num = pGameServer->m_World.FindEntities(ProjStartPos, pChr->GetProximityRadius() * 0.5f, (CEntity * *)apEnts,
			MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

		//for all found chars do
		for (int i = 0; i < Num; ++i)
		{
			//friendly fire heal
			CCharacter* pTarget = apEnts[i];
			if (pTarget->IsZombie() && pTarget != pChr && pTarget->GetHealthArmorSum() < 20) {
				pTarget->IncreaseOverallHp(4);
				pChr->IncreaseOverallHp(1);
				pTarget->SetEmote(EMOTE_HAPPY, pChr->Server()->Tick() + pChr->Server()->TickSpeed());
			}
			
			//exceptions
			if ((pTarget == pChr) || pGameServer->Collision()->IntersectLine(ProjStartPos, pTarget->GetPos(), NULL, NULL))
				continue;

			// set his velocity to fast upward (for now)
			if (length(pTarget->GetPos() - ProjStartPos) > 0.0f)
				pGameServer->CreateHammerHit(pTarget->GetPos() - normalize(pTarget->GetPos() - ProjStartPos) * pChr->GetProximityRadius() * 0.5f);
			else
				pGameServer->CreateHammerHit(ProjStartPos);

			//direction calculation
			vec2 Dir;
			if (length(pTarget->GetPos() - pChr->GetPos()) > 0.0f)
				Dir = normalize(pTarget->GetPos() - pChr->GetPos());
			else
				Dir = vec2(0.f, -1.f);

			//hammerfly related
			if (pTarget->IsZombie() && !pTarget->IsHookProtected()) {
				const int DAMAGE = 0; // 0 = no damage, TakeDamage() is called below just for hammerfly etc
				pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, Dir * -1, DAMAGE,
					pChr->GetPlayer()->GetCID(), pChr->GetActiveWeapon());
			}
			
			//infection
			if (pTarget->IsHuman())
				pTarget->Infect(ClientID);
			Hits++;
		}

		// if we Hit anything, we have to wait for the reload
		if (Hits)
			pChr->SetReloadTimer(pChr->Server()->TickSpeed() / 3);
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

