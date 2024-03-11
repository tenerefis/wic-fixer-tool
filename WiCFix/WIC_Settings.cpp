#include "stdafx.h"

WIC_Settings::WIC_Settings()
{
	this->m_RegistryKey = L"Software\\Massgate.org Installer\\World in Conflict";
	this->Clear();
}

WIC_Settings::~WIC_Settings()
{
}

void WIC_Settings::Clear()
{
	memset(this->myInstallExePath, 0, sizeof(this->myInstallExePath));
	memset(this->myInstallPath, 0, sizeof(this->myInstallPath));
	memset(this->myVersion, 0, sizeof(this->myVersion));
	memset(this->myWiCEdInstallPath, 0, sizeof(this->myWiCEdInstallPath));
	memset(this->myWiCEdVersion, 0, sizeof(this->myWiCEdVersion));
	memset(this->myWiCModKitInstallPath, 0, sizeof(this->myWiCModKitInstallPath));
	memset(this->myWiCModKitVersion, 0, sizeof(this->myWiCModKitVersion));
	memset(this->myWiCFixVersion, 0, sizeof(this->myWiCFixVersion));
	this->wicInstallDirFound = false;
	this->registryFixed = false;
	this->mpFixInstalled = false;
	this->mapsInstalled = false;
	this->wicautoexecInstalled = false;
	this->myDX10FlagSet = false;
	this->myWiCEdInstalled = false;
	this->myWiCModKitInstalled = false;
}

void WIC_Settings::Delete()
{
	// does not delete "Massgate.org Installer" key
	delete_reg_key(HKEY_CURRENT_USER, L"Software\\Massgate.org Installer");
}

void WIC_Settings::Load()
{
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myInstallExePath", this->myInstallExePath);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myInstallPath", this->myInstallPath);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myVersion", this->myVersion);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdInstallPath", this->myWiCEdInstallPath);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdVersion", this->myWiCEdVersion);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitInstallPath", this->myWiCModKitInstallPath);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitVersion", this->myWiCModKitVersion);
	read_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCFixVersion", this->myWiCFixVersion);
	this->wicInstallDirFound = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"wicInstallDirFound");
	this->registryFixed = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"registryFixed");
	this->mpFixInstalled = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"mpFixInstalled");
	this->mapsInstalled = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"mapsInstalled");
	this->wicautoexecInstalled = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"wicautoexecInstalled");
	this->myDX10FlagSet = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myDX10FlagSet");
	this->myWiCEdInstalled = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdInstalled");
	this->myWiCModKitInstalled = read_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitInstalled");
}

void WIC_Settings::Save()
{
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myInstallExePath", this->myInstallExePath);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myInstallPath", this->myInstallPath);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myVersion", this->myVersion);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdInstallPath", this->myWiCEdInstallPath);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdVersion", this->myWiCEdVersion);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitInstallPath", this->myWiCModKitInstallPath);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitVersion", this->myWiCModKitVersion);
	write_reg_wstring(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCFixVersion", this->myWiCFixVersion);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"wicInstallDirFound", this->wicInstallDirFound);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"registryFixed", this->registryFixed);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"mpFixInstalled", this->mpFixInstalled);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"mapsInstalled", this->mapsInstalled);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"wicautoexecInstalled", this->wicautoexecInstalled);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myDX10FlagSet", this->myDX10FlagSet);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCEdInstalled", this->myWiCEdInstalled);
	write_reg_dword(HKEY_CURRENT_USER, this->m_RegistryKey, L"myWiCModKitInstalled", this->myWiCModKitInstalled);
}

void WIC_Settings::Reset()
{
	this->Clear();
	this->Save();
}

bool WIC_Settings::isFixInstalled()
{
	return this->wicInstallDirFound && this->mpFixInstalled;
}