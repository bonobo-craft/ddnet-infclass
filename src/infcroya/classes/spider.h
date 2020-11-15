#pragma once

#include "class.h"

class CSpider : public IClass {
public:
	CSpider();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void Tick(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
	void ItFastJumps(CCharacter* pChr);
};
