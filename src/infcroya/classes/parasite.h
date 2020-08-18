#pragma once

#include "class.h"

class CParasite: public IClass {
public:
	CParasite();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;

    int OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon) override;  

};
