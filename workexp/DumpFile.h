#pragma once
#include <windows.h>
#include <Dbghelp.h>
#include <iostream>  
#include <vector>  
#include <tchar.h>
#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>

using namespace std; 


#pragma comment(lib, "Dbghelp.lib")

//void CreateMiniDump( EXCEPTION_POINTERS* pep ); 
//
//BOOL CALLBACK MyMiniDumpCallback(
//	PVOID                            pParam, 
//	const PMINIDUMP_CALLBACK_INPUT   pInput, 
//	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
//	); 
//
//bool IsDataSectionNeeded( const WCHAR* pModuleName ); 


namespace NSDumpFile
{ 
	///////////////////////////////////////////////////////////////////////////////
	// This function determines whether we need data sections of the given module 
	//

	bool IsDataSectionNeeded( const WCHAR* pModuleName ) 
	{
		// Check parameters 

		if( pModuleName == 0 ) 
		{
			_ASSERTE( _T("Parameter is null.") ); 
			return false; 
		}

		// Extract the module name 
		WCHAR szFileName[_MAX_FNAME] = L""; 

		_wsplitpath( pModuleName, NULL, NULL, szFileName, NULL ); 


		// Compare the name with the list of known names and decide 
		// Note: For this to work, the executable name must be "mididump.exe"
		if( wcsicmp( szFileName, L"mididump" ) == 0 ) 
		{
			return true; 
		}
		else if( wcsicmp( szFileName, L"ntdll" ) == 0 ) 
		{
			return true; 
		}

		// Complete 
		return false; 

	}

	BOOL CALLBACK MyMiniDumpCallback(
		PVOID                            pParam, 
		const PMINIDUMP_CALLBACK_INPUT   pInput, 
		PMINIDUMP_CALLBACK_OUTPUT        pOutput 
		) 
	{
		BOOL bRet = FALSE; 

		// Check parameters 
		if( pInput == 0 ) 
			return FALSE; 

		if( pOutput == 0 ) 
			return FALSE; 

		// Process the callbacks 
		switch( pInput->CallbackType ) 
		{
		case IncludeModuleCallback: 
			{
				// Include the module into the dump 
				bRet = TRUE; 
			}
			break; 

		case IncludeThreadCallback: 
			{
				// Include the thread into the dump 
				bRet = TRUE; 
			}
			break; 

		case ModuleCallback: 
			{
				// Are data sections available for this module ? 
				if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg ) 
				{
					// Yes, they are, but do we need them? 
					if( !IsDataSectionNeeded( pInput->Module.FullPath ) ) 
					{
						wprintf( L"Excluding module data sections: %s \n", pInput->Module.FullPath );
						pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 
					}
				}

				bRet = TRUE; 
			}
			break; 

		case ThreadCallback: 
			{
				// Include all thread information into the minidump 
				bRet = TRUE;  
			}
			break; 

		case ThreadExCallback: 
			{
				// Include this information 
				bRet = TRUE;  
			}
			break; 

		case MemoryCallback: 
			{
				// We do not include any information here -> return FALSE 
				bRet = FALSE; 
			}
			break; 

		case CancelCallback: 
			break; 
		}

		return bRet; 
	}
	
    void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)  
    {  
		// Open the file 

		HANDLE hFile = CreateFile( _T("MidiDump.dmp"), GENERIC_READ | GENERIC_WRITE, 
			0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

		if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
		{
			// Create the minidump 

			MINIDUMP_EXCEPTION_INFORMATION mdei; 

			mdei.ThreadId           = GetCurrentThreadId(); 
			mdei.ExceptionPointers  = pException; 
			mdei.ClientPointers     = FALSE; 

			MINIDUMP_CALLBACK_INFORMATION mci; 

			mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
			mci.CallbackParam       = 0; 

			MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
				MiniDumpWithDataSegs | 
				MiniDumpWithHandleData |
				MiniDumpWithFullMemoryInfo | 
				MiniDumpWithThreadInfo | 
				MiniDumpWithUnloadedModules ); 

			BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
				hFile, mdt, (pException != 0) ? &mdei : 0, 0, &mci ); 

			if( !rv ) 
				_tprintf( _T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError() ); 
			else 
				_tprintf( _T("Minidump created.\n") ); 

			// Close the file 

			CloseHandle( hFile ); 

		}
		else 
		{
			_tprintf( _T("CreateFile failed. Error: %u \n"), GetLastError() ); 
		}
    }  


    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
    {
        return NULL;
    }


    BOOL PreventSetUnhandledExceptionFilter()
    {
        HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
        if (hKernel32 ==   NULL)
            return FALSE;


        void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
        if(pOrgEntry == NULL)
            return FALSE;


        unsigned char newJump[ 100 ];
        DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
        dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far


        void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
        DWORD dwNewEntryAddr = (DWORD) pNewFunc;
        DWORD dwRelativeAddr = dwNewEntryAddr -  dwOrgEntryAddr;


        newJump[ 0 ] = 0xE9;  // JMP absolute
        memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));
        SIZE_T bytesWritten;
        BOOL bRet = WriteProcessMemory(GetCurrentProcess(),    pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
        return bRet;
    }


    LONG WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS *pException)
    {
        TCHAR szMbsFile[MAX_PATH] = { 0 };
        ::GetModuleFileName(NULL, szMbsFile, MAX_PATH);
        TCHAR* pFind = _tcsrchr(szMbsFile, '\\');
        if(pFind)
        {
            *(pFind+1) = 0;
            _tcscat(szMbsFile, _T("CrashDumpFile.dmp"));
            CreateDumpFile((LPCWSTR)szMbsFile, pException);
        }


        // TODO: MiniDumpWriteDump
        FatalAppExit(-1,  _T("Fatal Error"));
        return EXCEPTION_CONTINUE_SEARCH;
    }


    void RunCrashHandler()
    {
        SetUnhandledExceptionFilter(UnhandledExceptionFilterEx);
        PreventSetUnhandledExceptionFilter();
    }

};


#define DeclareDumpFile() NSDumpFile::RunCrashHandler();