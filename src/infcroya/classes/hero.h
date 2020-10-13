#pragma once

#include "class.h"

class CHero : public IClass {
public:
	CHero();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
	void GunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction) override;

};