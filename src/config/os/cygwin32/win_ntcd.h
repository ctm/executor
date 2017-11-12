/*
		Interface to cdenable.sys driver.
		11/15/1997 Lauri Pesonen
*/

/*
		Installs the driver, if not already installed.
		Starts the driver, if not already running.

		You can either always call "CdenableSysInstallStart" when your 
		program fires up and "CdenableSysStopRemove" when it terminates,
		or just let the installation program call "CdenableSysInstallStart"
		and leave it always be present. 

		I recommend the latter option. Calling "CdenableSysInstallStart"
		always doesn't hurt anything, it will immediately return
		with success if the service is running.

		Returns non-zero if installation/startup was succesfull,
		zero if anything failed.
		Returns non-zero also if the driver was already running.

		The file "cdenable.sys" must already have been copied to
		the directory "System32\Drivers"
*/
BOOL CdenableSysInstallStart(void);

/*
		Stops and removes the driver. See above.
		This must be called when new version of the driver is updated.
*/
void CdenableSysStopRemove(void);

/*
		HANDLE h: returned from CreateFile ( "\\\\.\\X:", GENERIC_READ, ... );
		Returns the bytes actually read (==count), 0 on failure.
		NOTE: in my code, start and count are always aligned to
		sector boundaries (2048 bytes). 
		I cannot guarantee that this works if they are not.
		Max read is 64 kb.
		Synchronous read, but quite fast.
*/
int CdenableSysReadCdBytes(HANDLE h, DWORD start, DWORD count, char *buf);

/*
		Same as SysReadCdBytes, but "start" and "count" are in 2048 byte
		sectors.
*/
int CdenableSysReadCdSectors(HANDLE h, DWORD start, DWORD count, char *buf);

/*
		Returns CDENABLE_CURRENT_VERSION (of the driver).
*/
DWORD CdenableSysGetVersion(void);
