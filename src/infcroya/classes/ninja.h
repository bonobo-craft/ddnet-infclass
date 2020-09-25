#pragma once

#include "class.h"

class CNinja : public IClass {
public:
	CNinja();

	void InitialWeaponsHealth(class CCharacter* pChr) override;
	void OnMouseWheelDown(class CCharacter* pChr) override;
	void OnMouseWheelUp(class CCharacter* pChr) override;
	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
};