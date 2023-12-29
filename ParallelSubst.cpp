#define _CRT_SECURE_NO_WARNINGS
#define UNICODE

#include <stdio.h>
#include <windows.h>

#define SUBST_PATH L"C:\\Windows\\System32\\subst.exe"
#define SUBST_DRIVE_LETTER L"K:"
#define ITERATIONS 100
#define SUBST_INSTANCES 4

int main()
{
    TCHAR tmp_path[MAX_PATH];
    DWORD tp_len = GetTempPath(MAX_PATH, tmp_path);

    TCHAR subst_dir_paths[SUBST_INSTANCES][MAX_PATH];
    for (int i = 0; i < SUBST_INSTANCES; ++i)
    {
        _snwprintf(subst_dir_paths[i], MAX_PATH, L"%ssubst.%d", tmp_path, i);
        CreateDirectory(subst_dir_paths[i], NULL);
    }

    int num_succeeded[SUBST_INSTANCES + 1] = { 0 };

    for (int i = 0; i < ITERATIONS; ++i)
    {
        /* Launch SUBST_INSTANCES (suspended) `subst X: <unique temporary folder>`, then resume
         * them all at once (or as close as we can get, anyway).
        */

        PROCESS_INFORMATION procs[SUBST_INSTANCES];
        TCHAR cmdline[MAX_PATH + 16];

        STARTUPINFO si;
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);

        for (int j = 0; j < SUBST_INSTANCES; ++j)
        {
            _snwprintf(cmdline, (MAX_PATH + 16), L"subst %s %s", SUBST_DRIVE_LETTER, subst_dir_paths[j]);

            if (!CreateProcess(
                SUBST_PATH,
                cmdline,
                NULL,
                NULL,
                FALSE,
                CREATE_SUSPENDED,
                NULL,
                NULL,
                &si,
                &(procs[j])))
            {
                fprintf(stderr, "Failed to create subst.exe process\n");
                return 1;
            }
        }

        for (int j = 0; j < SUBST_INSTANCES; ++j)
        {
            ResumeThread(procs[j].hThread);
        }

        /* Wait for the instances to exit and add up how many claimed success. */

        int num_ok = 0;
        for (int j = 0; j < SUBST_INSTANCES; ++j)
        {
            WaitForSingleObject(procs[j].hProcess, INFINITE);

            DWORD exit_code;
            GetExitCodeProcess(procs[j].hProcess, &exit_code);

            CloseHandle(procs[j].hThread);
            CloseHandle(procs[j].hProcess);

            if (exit_code == 0)
            {
                ++num_ok;
            }
        }

        ++num_succeeded[num_ok];

        /* Run `subst X: /D` to remove the drive mapping. */

        _snwprintf(cmdline, (MAX_PATH + 16), L"subst %s /D", SUBST_DRIVE_LETTER);

        if (!CreateProcess(
            SUBST_PATH,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &(procs[0])))
        {
            fprintf(stderr, "Failed to create subst.exe process\n");
            return 1;
        }

        WaitForSingleObject(procs[0].hProcess, INFINITE);
        CloseHandle(procs[0].hThread);
        CloseHandle(procs[0].hProcess);
    }

    printf("Ran %d instances of subst.exe in parallel %d times\n", SUBST_INSTANCES, ITERATIONS);
    
    for (int i = 0; i < SUBST_INSTANCES + 1; ++i)
    {
        printf("In %d iterations, %d of the subst processes succeeded\n", num_succeeded[i], i);
    }

    return 0;
}
