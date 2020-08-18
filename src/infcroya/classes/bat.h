#pragma once

#include "class.h"

class CBat : public IClass {
public:
	CBat();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void Tick(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
};
