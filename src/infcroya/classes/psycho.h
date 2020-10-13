#pragma once

#include "class.h"

class CPsycho : public IClass {
public:
	CPsycho();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;

};