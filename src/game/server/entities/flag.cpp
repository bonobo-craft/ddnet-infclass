/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "flag.h"

CFlag::CFlag(CGameWorld *pGameWorld, int Team, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, Pos)
{
	m_Team = Team;
	GameWorld()->InsertEntity(this);
	m_ProximityRadius = ms_PhysSize;
	m_pCarryingCharacter = NULL;
	m_GrabTick = 0;
	m_StandPos = Pos;
	m_Pos = Pos;

	Reset();
}

void CFlag::Reset()
{
	m_pCarryingCharacter = NULL;
	m_AtStand = 1;
	m_Pos = m_StandPos;
	m_Vel = vec2(0,0);
	m_GrabTick = 0;
}

void CFlag::TickPaused()
{
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
}

void CFlag::SetPos(vec2 newPos)
{
	m_Pos = newPos;
	m_StandPos = newPos;
}

void CFlag::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	if (Server()->IsSixup(SnappingClient)) {
		protocol7::CNetObj_Flag *pFlag = (protocol7::CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, m_Team, sizeof(protocol7::CNetObj_Flag));
		if(!pFlag)
			return;

		pFlag->m_X = (int)m_StandPos.x;
		pFlag->m_Y = (int)m_StandPos.y;
		pFlag->m_Team = 0;
	} else {
		CNetObj_Flag *pFlag = (CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, m_Team, sizeof(CNetObj_Flag));
		if(!pFlag)
			return;

		pFlag->m_X = (int)m_StandPos.x;
		pFlag->m_Y = (int)m_StandPos.y;
		pFlag->m_Team = 0;
	}
}
