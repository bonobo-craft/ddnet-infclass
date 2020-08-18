#pragma once

#include "class.h"

class CMother : public IClass {
public:
	CMother();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;

private:
	void MotherPlaceRespawn(class CCharacter* pChr);

};
