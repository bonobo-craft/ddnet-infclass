#pragma once

#include <infcroya/skin.h>
#include <base/vmath.h>
#include <unordered_map>
#include <string>
#include <game/server/entities/character.h>

class IClass {
private:
	CSkin m_Skin;
	bool m_Infected;
public:
	void AntigravityOn(CCharacter* pChr, bool LooseSpeed = true);
	void AntigravityOff(CCharacter* pChr);
	void ItAntigravitates(CCharacter* pChr);
	int AntigravityTimeLeft;
    char m_06SkinName[64] = {'\0'};
	int m_06SkinBodyColor = 0;
	int m_06SkinFeetColor = 0;
	void Set06SkinName(const char* name);
	void Set06SkinColors(int SkinBodyColor06, int SkinFeetColor06);
	std::string m_Name;
	static std::unordered_map<int, class IClass*> classes;
	virtual ~IClass();

	virtual void InitialWeaponsHealth(class CCharacter* pChr) = 0;

	virtual void Tick(class CCharacter* pChr);

	virtual void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) = 0;
	virtual void OnMouseWheelDown(class CCharacter* pChr);
	virtual void OnMouseWheelUp(class CCharacter* pChr);

	virtual void OnCharacterSpawn(class CCharacter* pChr);
	virtual int OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon);

	virtual const CSkin& GetSkin() const;
	virtual void SetSkin(CSkin skin);

	virtual bool IsInfectedClass() const;
	virtual void SetInfectedClass(bool Infected);

	virtual std::string GetName() const;
	virtual void SetName(std::string Name);

	virtual void GunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction);
	void ShotgunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction);
	void GrenadeShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction);
	void HammerShoot(CCharacter* pChr, vec2 ProjStartPos);
	void ItDoubleJumps(CCharacter* pChr);
	void ItSelfAntigravitates(CCharacter* pChr);
	virtual void UnlockPosition(CCharacter* pChr);

	virtual bool WillItFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr);
};

enum Class {
	HUMAN_CLASS_START = 0,
	DEFAULT,
	BIOLOGIST,
	ENGINEER,
	MEDIC,
	SOLDIER,
	SCIENTIST,
	MERCENARY,
	NINJA,
	HERO,
	SNIPER,
	PSYCHO,
	LOOPER,
	MAGICIAN,
	HUMAN_CLASS_END,

	ZOMBIE_CLASS_START,
	SMOKER,
	HUNTER,
	BAT,
	FREEZER,
	WORKER,
	BOOMER,
	POISONER,
	SPIDER,
    MOTHER,
	PARASITE,
	ZOMBIE_CLASS_END,
};
