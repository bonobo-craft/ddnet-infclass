#pragma once

#include "class.h"

class CSniper : public IClass {
public:
	CSniper();

	int LockPositionTimeLeft;
	void InitialWeaponsHealth(class CCharacter* pChr) override;
	void Tick(CCharacter* pChr) override;
	void ItLocksInSpace(CCharacter* pChr);
	void LockOrUnlockPosition(CCharacter* pChr);
	void LockPosition(CCharacter* pChr);
	void UnlockPosition(CCharacter* pChr) override;
	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
};