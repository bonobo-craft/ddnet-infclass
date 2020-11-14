#pragma once

#include "class.h"

class CMagician : public IClass {
public:
	CMagician();

	int LockPositionTimeLeft;
	void InitialWeaponsHealth(class CCharacter* pChr) override;
	void Tick(CCharacter* pChr) override;
	void ItLocksInSpace(CCharacter* pChr);
	void LockOrUnlockPosition(CCharacter* pChr);
	void LockPosition(CCharacter* pChr);
	void UnlockPosition(CCharacter* pChr) override;
	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;
	void GunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction) override;
};