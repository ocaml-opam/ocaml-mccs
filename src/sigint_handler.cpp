#include <cudf.h>

#ifndef _WIN32

static void sigint_handler(int sig, siginfo_t *si, void * ucontext) {
  throw 130;
}

static struct sigaction ocaml_sigint_action;

void install_sigint_handler() {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = sigint_handler;
  if (sigaction(SIGINT, &sa, &ocaml_sigint_action) == -1) {
    PRINT_ERR("ERROR: cannot install solver signal handler\n");
    exit(99);
  }
  return;
}

void restore_sigint_handler() {
  if (sigaction (SIGINT, &ocaml_sigint_action, NULL) == -1) {
    PRINT_ERR("ERROR: cannot restore solver signal handler\n");
    exit(99);
  }
  return;
}

#else

#include <Windows.h>

/*
 * Handling CTRL+C on Windows. Windows allows a handler function to be called on
 * certain events via the SetConsoleCtrlHandler function. This function handles
 * both the addition and removal of handler functions and maintains a stack of
 * handlers (so there is no need to back-up OCaml's handler).
 *
 * When CTRL+C is received, Windows will create a new thread in which to execute
 * the handler, so an exception cannot simply br thrown. The solution here is
 * to store a handle to the solver thread in the global hExecutionThread before
 * the handler installed. The handler then pauses the solver thread and rewrites
 * its context, sending it into a function which will throw the required
 * exception.
 *
 * See https://www.codeproject.com/Articles/71529/Exception-Injection-Throwing-an-Exception-in-Other
 * for the original idea (including a pointer to the amd64 variant in the
 * comments) and my thanks to Xavier Leroy for confirming that the details of
 * the amd64 context rewriting are indeed correct.
 */

static HANDLE hExecutionThread = NULL;
static bool bTerminating = false;
static HANDLE hMutex = NULL;

__declspec(noreturn) static void solver_terminate(void) {
  throw 130;
}

static BOOL WINAPI sigint_handler(DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT) {
    // The handler may be called multiple times if the user presses and holds
    // CTRL+C! Ensure that the exception will definitely only be thrown once.
    WaitForSingleObject(hMutex, INFINITE);
    if (bTerminating) {
      ReleaseMutex(hMutex);
      return TRUE;
    } else {
      bTerminating = true;
    }
    if (SuspendThread(hExecutionThread) != INFINITE) {
      CONTEXT ctx;
      ctx.ContextFlags = CONTEXT_CONTROL;
      if (GetThreadContext(hExecutionThread, &ctx)) {
#if defined(_M_IX86)
        // x86: simply move eip into the actual handler
        ctx.Eip = (DWORD)solver_terminate;
#elif defined(_M_AMD64)
        // amd64: move rip into the handler, ensure that the stack pointer is
        //        aligned to a 16-byte boundary and reserve 8 bytes (as per the
        //        function's prologue).
        ctx.Rip = (DWORD64)solver_terminate;
        ctx.Rsp &= -16;
        ctx.Rsp -= 8;
#else
#error "Unsupported CPU architecture"
#endif
        SetThreadContext(hExecutionThread, &ctx);
      }
      ResumeThread(hExecutionThread);
    }
    ReleaseMutex(hMutex);
    return TRUE;
  } else {
    return FALSE;
  }
}

void install_sigint_handler() {
  HANDLE hProcess = GetCurrentProcess();
  if (hMutex == NULL) {
    hMutex = CreateMutex(NULL, FALSE, NULL);
  }
  bTerminating = false;
  if (!DuplicateHandle(hProcess, GetCurrentThread(), hProcess, &hExecutionThread, 0, FALSE, DUPLICATE_SAME_ACCESS) ||
      !SetConsoleCtrlHandler(&sigint_handler, TRUE)) {
    PRINT_ERR("ERROR: cannot install solver signal handler\n");
    exit(99);
  }
  return;
}

void restore_sigint_handler() {
  WaitForSingleObject(hMutex, INFINITE);
  if (hExecutionThread) {
    CloseHandle(hExecutionThread);
    hExecutionThread = NULL;
  }
  bTerminating = true;
  ReleaseMutex(hMutex);
  if (!SetConsoleCtrlHandler(&sigint_handler, FALSE)) {
    PRINT_ERR("ERROR: cannot restore solver signal handler\n");
    exit(99);
  }
  return;
}

#endif
