/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/seh.h"
#include <stdlib.h>

#ifdef  OS_WINDOWS

#define _WIN32_WINDOWS	0x0500
#include <windows.h>
#include <winbase.h>
#ifdef _UNICODE 
#define _T(x)      L ## x
#else /* _UNICODE */
#define _T(x)      x
#endif /* _UNICODE */


namespace cxx {
    namespace sys {
        namespace seh {

            LONG shutdown::handler(const char *, PEXCEPTION_POINTERS)
            {
				exit(0);
                return EXCEPTION_EXECUTE_HANDLER;
            }

            ////////////////////////////////////////////////////////////////////
            // 记录异常信息。在处理中，尽量避免使用 CRT，而是使用系统函数，例如 WriteFile,
            // wvsprintf, lstrcpy 等，而不是 fwrite 等等。
            static int          hprintf_index        = 0;
            static const int    hprintf_buffer_size  = 8 * 1024;
            static TCHAR        hprintf_buffer[hprintf_buffer_size];
            static const int    ONEK                 = 1024;
            static const int    ONEM                 = ONEK*ONEK;

            static void hflush(HANDLE handle)
            {
                if(hprintf_index > 0) {
                    DWORD bytes;
                    WriteFile(handle, hprintf_buffer, lstrlen(hprintf_buffer),
                              &bytes, 0);
                    hprintf_index = 0;
                }
            }

            static void hprintf(HANDLE handle, LPCTSTR format, ...)
            {
                if(hprintf_index > hprintf_buffer_size - 1024) {
                    DWORD bytes;
                    WriteFile(handle, hprintf_buffer, lstrlen(hprintf_buffer),
                              &bytes, 0);
                    hprintf_index = 0;
                }

                va_list arg_list;
                va_start(arg_list, format);
                hprintf_index += wvsprintf(&hprintf_buffer[hprintf_index], format, arg_list);
                va_end(arg_list);
            }

            static void FormatTime(LPTSTR output, FILETIME ft)
            {
                WORD date, time;
                if(FileTimeToLocalFileTime(&ft, &ft) && FileTimeToDosDateTime(&ft, &date, &time)) {
                    wsprintf(output, _T("%d-%d-%d %02d:%02d:%02d"),
                             (date / 512) + 1980, (date/32) & 15, date & 31, 
                             (time >> 11), (time >> 5) & 0x3f, (time & 0x1f) * 2);
                }
            }

			static TCHAR* lstrrchr(LPCTSTR string, int ch)
			{
				TCHAR* start = (TCHAR* )string;
				while(*string++)
					;
				while(--string != start && *string != (TCHAR)ch)
					;
				if(*string == (TCHAR)ch)
					return (TCHAR* )string;
				return NULL;
			}

			static TCHAR* GetFilePart(LPCTSTR source)
			{
				TCHAR* result = lstrrchr(source, _T('\\'));
				if(result)
					result++;
				else
					result = (TCHAR* )source;
				return result;
			}

			static TCHAR* GetExceptionDescription(DWORD ExceptionCode)
			{
				struct ExceptionNames {
					DWORD	code;
					TCHAR*	name;
				};
				static const ExceptionNames ExceptionMap[] = {
					{0x40010005, _T("a Control-C")},
					{0x40010008, _T("a Control-Break")},
					{0x80000002, _T("a Datatype Misalignment")},
					{0x80000003, _T("a Breakpoint")},
					{0xc0000005, _T("an Access Violation")},
					{0xc0000006, _T("an In Page Error")},
					{0xc0000017, _T("a No Memory")},
					{0xc000001d, _T("an Illegal Instruction")},
					{0xc0000025, _T("a Noncontinuable Exception")},
					{0xc0000026, _T("an Invalid Disposition")},
					{0xc000008c, _T("a Array Bounds Exceeded")},
					{0xc000008d, _T("a Float Denormal Operand")},
					{0xc000008e, _T("a Float Divide by Zero")},
					{0xc000008f, _T("a Float Inexact Result")},
					{0xc0000090, _T("a Float Invalid Operation")},
					{0xc0000091, _T("a Float Overflow")},
					{0xc0000092, _T("a Float Stack Check")},
					{0xc0000093, _T("a Float Underflow")},
					{0xc0000094, _T("an Integer Divide by Zero")},
					{0xc000008f, _T("a Float Inexact Result")},
					{0xc0000090, _T("a Float Invalid Operation")},
					{0xc0000091, _T("a Float Overflow")},
					{0xc0000092, _T("a Float Stack Check")},
					{0xc0000093, _T("a Float Underflow")},
					{0xc0000094, _T("an Integer Divide by Zero")},
					{0xc0000095, _T("an Integer Overflow")},
					{0xc0000096, _T("a Privileged Instruction")},
					{0xc00000fD, _T("a Stack Overflow")},
					{0xc0000142, _T("a DLL Initialization Failed")},
					{0xe06d7363, _T("a Microsoft C++ Exception")},
				};

				for(int i = 0; i < sizeof(ExceptionMap)/sizeof(ExceptionMap[0]); ++i) {
					if(ExceptionCode == ExceptionMap[i].code)
						return ExceptionMap[i].name;
				}
				return _T("an Unknown exception type");
			}

            static void DumpSystemInformation(HANDLE log_file)
            {
                // 打印当前时间
                FILETIME    current_time;
                GetSystemTimeAsFileTime(&current_time);
                TCHAR       time_buffer[100];
                FormatTime(time_buffer, current_time);

                hprintf(log_file, _T("Error occured at %s\r\n"), time_buffer);

                // 打印程序名和用户名
                TCHAR  module_name[MAX_PATH * 2];
                ZeroMemory(module_name, sizeof(module_name));
                if(GetModuleFileName(0, module_name, sizeof(module_name) - 2) <= 0) {
                    lstrcpy(module_name, _T("unknown"));
                }
                TCHAR  user_name[200];
                ZeroMemory(user_name, sizeof(user_name));
                DWORD  user_name_size = sizeof(user_name) - 2;
                if(!GetUserName(user_name, &user_name_size)) {
                    lstrcpy(user_name, _T("unknown"));
                }

                hprintf(log_file, _T("%s, run by %s\r\n"), module_name, user_name);

                // 打印操作系统信息
                OSVERSIONINFO   verinfo;
				verinfo.dwOSVersionInfoSize = sizeof(verinfo);
				GetVersionEx(&verinfo);
                hprintf(log_file, _T("Operation System: %u.%u.%u.%u\r\n"), 
					verinfo.dwPlatformId, verinfo.dwMajorVersion, 
					verinfo.dwMinorVersion, verinfo.dwBuildNumber & 0xFFFF);

                // 打印系统信息
                SYSTEM_INFO     sysinfo;
                GetSystemInfo(&sysinfo);
                hprintf(log_file, _T("%d processor(s), type %d\r\n"),
                                     sysinfo.dwNumberOfProcessors, sysinfo.dwProcessorType);
                MEMORYSTATUS    meminfo;
                meminfo.dwLength = sizeof(meminfo);
                GlobalMemoryStatus(&meminfo);
                hprintf(log_file, _T("%d%% memory in use\r\n"), meminfo.dwMemoryLoad);
                hprintf(log_file, _T("%d MBytes physical memory\r\n"), (meminfo.dwTotalPhys + ONEM - 1)/ONEM);
                hprintf(log_file, _T("%d MBytes physical memory free\r\n"), (meminfo.dwAvailPhys + ONEM - 1)/ONEM);
                hprintf(log_file, _T("%d MBytes paging file\r\n"), (meminfo.dwTotalPageFile + ONEM - 1)/ONEM);
                hprintf(log_file, _T("%d MBytes paging file free\r\n"), (meminfo.dwAvailPageFile + ONEM - 1)/ONEM);
                hprintf(log_file, _T("%d MBytes user address space\r\n"), (meminfo.dwTotalVirtual + ONEM - 1)/ONEM);
                hprintf(log_file, _T("%d MBytes user address space free\r\n"), (meminfo.dwAvailVirtual + ONEM - 1)/ONEM);
            }

            static void DumpRegisters(HANDLE log_file, PCONTEXT context)
            {
                hprintf(log_file, _T("\r\n"));
                hprintf(log_file, _T("Context:\r\n"));
#if	defined(_X86_)
                hprintf(log_file, _T("EDI:    0X%08X  ESI: 0X%08X  EAX: 0X%08X\r\n"),
                        context->Edi, context->Esi, context->Eax);
                hprintf(log_file, _T("EBX:    0X%08X  ECX: 0X%08X  EDX: 0X%08X\r\n"),
                        context->Ebx, context->Ecx, context->Edx);
                hprintf(log_file, _T("EIP:    0X%08X  EBP: 0X%08X  SegCs: 0X%08X\r\n"),
                        context->Eip, context->Ebp, context->SegCs);
                hprintf(log_file, _T("EFlags: 0X%08X  ESP: 0X%08X  SegSs: 0X%08X\r\n"),
                        context->EFlags, context->Esp, context->SegSs);
#elif defined(_AMD64_)
				hprintf(log_file, _T("RDI:    0X%016I64X  RSI: 0X%016I64X  RAX: 0X%016I64X\r\n"),
                        context->Rdi, context->Rsi, context->Rax);
                hprintf(log_file, _T("RBX:    0X%016I64X  RCX: 0X%016I64X  RDX: 0X%016I64X\r\n"),
                        context->Rbx, context->Rcx, context->Rdx);
                hprintf(log_file, _T("RIP:    0X%016I64X  RBP: 0X%016I64X  SegCs: 0X%08X\r\n"),
                        context->Rip, context->Rbp, context->SegCs);
                hprintf(log_file, _T("EFlags: 0X%08X  RSP: 0X%016I64X  SegSs: 0X%08X\r\n"),
                        context->EFlags, context->Rsp, context->SegSs);
#else
#error "unknown processor arch"
#endif
            }

            static void DumpStack(HANDLE log_file, DWORD* stack)
            {
                // TODO:
            }

            static void DumpModuleList(HANDLE log_file)
            {
                // TODO:
            }

            static void DumpMiniDump(HANDLE log_file, PEXCEPTION_POINTERS exp)
            {
                // TODO:
            }


            ////////////////////////////////////////////////////////////////////

            LONG report::handler(const char * file, PEXCEPTION_POINTERS exp)
            {
                static bool is_first_time = true;
                if(!is_first_time) {
                    // 多次进入，可能发生循环了。
                    return EXCEPTION_CONTINUE_SEARCH;
                }
                is_first_time = false;

                TCHAR  module_name[MAX_PATH * 2];
                TCHAR  file_name[MAX_PATH * 2];
                ZeroMemory(module_name, sizeof(module_name));
                if(GetModuleFileName(0, module_name, sizeof(module_name) - 2) <= 0) {
                    lstrcpy(module_name, _T("unknown"));
                }
                lstrcpy(file_name, GetFilePart(module_name));

                HANDLE log_file = CreateFile(file, GENERIC_WRITE, FILE_SHARE_READ, 0,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
                if(log_file == NULL) {
                    printf("creating log file %s failed\n", file);
                    return EXCEPTION_CONTINUE_SEARCH;
                }
                SetFilePointer(log_file, 0, 0, FILE_END);

                TCHAR  crashed_module_path[MAX_PATH * 2];
                TCHAR* crashed_module_file = _T("Unknown");
                ZeroMemory(crashed_module_path, sizeof(crashed_module_path));

                PEXCEPTION_RECORD           records = exp->ExceptionRecord;
                PCONTEXT                    context = exp->ContextRecord;
                MEMORY_BASIC_INFORMATION    meminfo;
#if	defined(_X86_)
                if(VirtualQuery((void* )context->Eip, &meminfo, sizeof(meminfo)) &&
                        (GetModuleFileName((HINSTANCE)meminfo.AllocationBase,
                                           crashed_module_path,
                                           sizeof(crashed_module_path) - 2) > 0)
                        ) {
                    crashed_module_file = GetFilePart(crashed_module_path);
                }

                hprintf(log_file, _T("%s caused %s (0X%08x) \r\nin module %s at %04X:%08X\r\n\r\n"),
                        file_name, GetExceptionDescription(records->ExceptionCode),
                        records->ExceptionCode, crashed_module_file,
                        context->SegCs, context->Eip);
#elif defined(_AMD64_)
				if(VirtualQuery((void* )context->Rip, &meminfo, sizeof(meminfo)) &&
                        (GetModuleFileName((HINSTANCE)meminfo.AllocationBase,
                                           crashed_module_path,
                                           sizeof(crashed_module_path) - 2) > 0)
                        ) {
                    crashed_module_file = GetFilePart(crashed_module_path);
                }

                hprintf(log_file, _T("%s caused %s (0X%08x) \r\nin module %s at %04X:%016I64X\r\n\r\n"),
                        file_name, GetExceptionDescription(records->ExceptionCode),
                        records->ExceptionCode, crashed_module_file,
                        context->SegCs, context->Rip);
#else
#error "unknown processor arch"
#endif

                DumpSystemInformation(log_file);

                if(records->ExceptionCode == STATUS_ACCESS_VIOLATION &&
                        records->NumberParameters >= 2) {
                    TCHAR  debug_message[1000];
                    TCHAR* readwrite = _T("Read from");
                    if(records->ExceptionInformation[0])
                        readwrite = _T("Write to");
                    wsprintf(debug_message, _T("%s location %08X caused an access violation\r\n"),
                             readwrite, records->ExceptionInformation[1]);
                    hprintf(log_file, _T("%s"), debug_message);
                }

                DumpRegisters(log_file, context);

                // 打印指令寄存器的代码内容。因为发生结构化异常时指令寄存器可能是错误的，因此
                // 访问其后续的内容可能会导致其他的异常。在这种情况下会打印??代替
#if	defined(_X86_)
                hprintf(log_file, _T("\r\nBytes at CS:EIP:\r\n"));
                unsigned char* code = (unsigned char* )context->Eip;
#elif	defined(_AMD64_)
                hprintf(log_file, _T("\r\nBytes at CS:RIP:\r\n"));
                unsigned char* code = (unsigned char* )context->Rip;
#else
#error "unknown processor arch"
#endif
                for(int codebyte = 0; codebyte < 16; ++codebyte) {
                    __try {
                        hprintf(log_file, _T("%02X "), code[codebyte]);
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) {
                        hprintf(log_file, _T("?? "));
                    }
                }

#if	defined(_X86_)
                DWORD* stack = (DWORD* )context->Esp;
#elif	defined(_AMD64_)
				DWORD* stack = (DWORD* )context->Rsp;
#else
#error "unknown processor arch"
#endif
                DumpStack(log_file, stack);

                DumpModuleList(log_file);

                hprintf(log_file, _T("\r\n====== [end of %s] ======\r\n"), file);
                hflush(log_file);
                CloseHandle(log_file);

                TCHAR  dmp_file_name[MAX_PATH * 2];
                lstrcpy(dmp_file_name, file);
                lstrcpy(dmp_file_name, ".dmp");

                HANDLE dmp_file = CreateFile(dmp_file_name, GENERIC_WRITE, FILE_SHARE_READ, 0,
                                             OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                             NULL);
                if(dmp_file != INVALID_HANDLE_VALUE) {
                    DumpMiniDump(dmp_file, exp);
                    CloseHandle(dmp_file);
                }

                return EXCEPTION_CONTINUE_SEARCH;
            }

        }
    }
}

#endif  /** OS_WINDOWS */
