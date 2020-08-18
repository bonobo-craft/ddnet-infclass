/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "poison-circle.h"
#include "growingexplosion.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <infcroya/classes/class.h>
#include <infcroya/croyaplayer.h>

CPoisonCircle::CPoisonCircle(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_POISON_CIRCLE, Pos)
{
	m_Pos = Pos;
	GameWorld()->InsertEntity(this);
	m_DetectionRadius = 60.0f;
	m_StartTick = Server()->Tick();
	m_Owner = Owner;
	
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CPoisonCircle::~CPoisonCircle()
{
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CPoisonCircle::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

int CPoisonCircle::GetOwner() const
{
	return m_Owner;
}


void CPoisonCircle::Snap(int SnappingClient)
{
	float Radius = g_Config.m_InfPoisonCircleRadius;
	
	for(int i=0; i<CPoisonCircle::NUM_PARTICLES; i++)
	{
		float RandomRadius = frandom()*(Radius-4.0f);
		float RandomAngle = 2.0f * pi * frandom();
		vec2 ParticlePos = m_Pos + vec2(RandomRadius * cos(RandomAngle), RandomRadius * sin(RandomAngle));
			
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[CPoisonCircle::NUM_SIDE+i], sizeof(CNetObj_Projectile)));
		if(pObj)
		{
			pObj->m_X = (int)ParticlePos.x;
			pObj->m_Y = (int)ParticlePos.y;
			pObj->m_VelX = 0;
			pObj->m_VelY = 0;
			pObj->m_StartTick = Server()->Tick();
			pObj->m_Type = WEAPON_HAMMER;
		}
	}

}

void CPoisonCircle::Tick()
{
	if(IsMarkedForDestroy()) return;

	// Find other players
	for(CCharacter *p = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(p->IsZombie()) continue;
		//if(p->GetClass() == PLAYERCLASS_UNDEAD && p->IsFrozen()) continue;
		//if(p->GetClass() == PLAYERCLASS_VOODOO && p->m_VoodooAboutToDie) continue;
		if(p->GetCroyaPlayer()->GetClassNum() == Class::BIOLOGIST || p->GetCroyaPlayer()->GetClassNum() == Class::HERO) {
			continue;
		}

		float Len = distance(p->GetPos(), m_Pos);
		if(Len < p->GetProximityRadius()+g_Config.m_InfPoisonCircleRadius)
		{
			CPlayer *pPoisonedBy = p->GetPlayer();

			if (pPoisonedBy)
				p->Poison(g_Config.m_InfPoisonCircleDamageSeconds, pPoisonedBy->GetCID());
			break;
		}
	}

}

void CPoisonCircle::TickPaused()
{
	++m_StartTick;
}
