#pragma once

#ifndef WIC_SETTINGS_H
#define WIC_SETTINGS_H

class WIC_Settings
{
private:
	const wchar_t *m_RegistryKey;

public:
	wchar_t myInstallExePath[MAX_STRING_LENGTH];
	wchar_t myInstallPath[MAX_STRING_LENGTH];
	wchar_t myVersion[MAX_STRING_LENGTH];
	wchar_t myWiCFixVersion[MAX_STRING_LENGTH];
	bool wicInstallDirFound;
	bool registryFixed;
	bool mpFixInstalled;
	bool mapsInstalled;
	bool wicautoexecInstalled;
	bool myDX10FlagSet;

private:
	void Clear();

public:
	WIC_Settings();
	~WIC_Settings();

	void Delete();
	void Load();
	void Save();
	void Reset();

	bool isFixInstalled();
};

#endif