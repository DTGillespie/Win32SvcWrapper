#include <windows.h>
#include <stdio.h>
#include <string>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void WINAPI SvcMain(DWORD argc, LPWSTR* argv);
void CtrlHandler(DWORD request);
void StartExe();
void StopExe();
void ReadConfig();
std::wstring CharArrayToLPCWSTR(const char* charArray);

PROCESS_INFORMATION pi;

char svcName[260], exePath[260];
std::wstring wSvcName;

int main() {

	ReadConfig();

	wSvcName = CharArrayToLPCWSTR(svcName);

	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = &wSvcName[0]; // Extract writable buffer wchar_t* from wSvcName
	ServiceTable[0].lpServiceProc = SvcMain; // Register SvcMain with SCM
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	// Start service
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

	std::wstring wExePath = CharArrayToLPCWSTR(exePath);

	if (!CreateProcess(&wExePath[0], NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) printf("Failed to start executable.\n");
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

void ReadConfig() {
	GetPrivateProfileStringA("Service", "Name", "DefaultService", svcName, sizeof(svcName), "config.ini");
	GetPrivateProfileStringA("Exe", "Path", "C:\\Path\\To\\DefaultApp.exe", exePath, sizeof(exePath), "config.ini");
}

// Convert char* to wstring
std::wstring CharArrayToLPCWSTR(const char* charArray) {

	int len = MultiByteToWideChar(CP_ACP, 0, charArray, -1, NULL, 0);
	if (len == 0) return L"";
	
	std::wstring wString(len, L'\0');

	MultiByteToWideChar(CP_ACP, 0, charArray, -1, &wString[0], len);

	return wString;
}