
#include <Windows.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#include<stdio.h>
#include<string.h>

void showProcessInformation() {
	DWORD dwProcID;		//ID
	HANDLE hProcess;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(hSnapshot) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if(Process32First(hSnapshot,&pe32)) {
            do {
               printf("pid %d %s\n",pe32.th32ProcessID,pe32.szExeFile);
                  if(strcmp(pe32.szExeFile,"ffplay.exe")==0)
                  {
                  	dwProcID=pe32.th32ProcessID;
					hProcess = OpenProcess(PROCESS_ALL_ACCESS,0, dwProcID);
					TerminateProcess(hProcess, 0);
				  }
               		
            } while(Process32Next(hSnapshot,&pe32));
         }
         CloseHandle(hSnapshot);
    }
}
int main()
{
	showProcessInformation();

}
