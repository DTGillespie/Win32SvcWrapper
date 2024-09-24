#include <windows.h>
#include <stdio.h>
#include <string>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
PROCESS_INFORMATION pi;

void WINAPI SvcMain(DWORD argc, LPWSTR* argv);
void CtrlHandler(DWORD request);
void StartExe();
void StopExe();
void SetWorkingDirectory();
void ReadConfig();

wchar_t wSvcName[260], wExePath[260];

int main() {

	SetWorkingDirectory();
	ReadConfig();

	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = wSvcName;
	ServiceTable[0].lpServiceProc = SvcMain; // Register SvcMain with SCM
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	StartServiceCtrlDispatcher(ServiceTable);

	return 0;
}

void WINAPI SvcMain(DWORD argc, LPWSTR* argv) {

	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler(&wSvcName[0], CtrlHandler);
	if (hStatus == (SERVICE_STATUS_HANDLE) 0) return;

	// Report starting status to SCM
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	SetServiceStatus(hStatus, &ServiceStatus);

	StartExe();

	// Report running status to SCM
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);

	// Keep service alive
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) Sleep(1000);

	StopExe();

	// Report stopped status to SCM
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hStatus, &ServiceStatus);
}

void CtrlHandler(DWORD request) {
	switch (request) {

		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:

			// Report pending-stop to SCM
			ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus(hStatus, &ServiceStatus);

			// Stop executable
			StopExe();

			// Report stopped status to SCM
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(hStatus, &ServiceStatus);

		return;

		default: break;
	}
}

void StartExe() {

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(wExePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) printf("Failed to start executable.\n");
}

void StopExe() {
	if (pi.hProcess) {
		
		TerminateProcess(pi.hProcess, 0);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		
		pi.hProcess = NULL;
		pi.hThread  = NULL;
	}
}

void SetWorkingDirectory() {
	char wd[MAX_PATH];
	GetModuleFileNameA(NULL, wd, MAX_PATH);

	char* lastBackslash = strrchr(wd, '\\');
	if (lastBackslash) {
		*lastBackslash = '\0';
		SetCurrentDirectoryA(wd);
	}
}

void ReadConfig() {
	wchar_t configPath[MAX_PATH];
	GetFullPathName(L"config.ini", MAX_PATH, configPath, NULL);

	GetPrivateProfileString(L"Service", L"Name", L"DefaultService", wSvcName, sizeof(wSvcName) / sizeof(wchar_t), configPath);
	GetPrivateProfileString(L"Service", L"Path", L"C:\\Path\\To\\DefaultApp.exe", wExePath, sizeof(wExePath) / sizeof(wchar_t), configPath);
}