#include "hooks_py.h"
#include "simulator.h"
#include "core_manager.h"
#include "config.hpp"
#include "fxsupport.h"

bool HooksPy::pyInit = false;

void HooksPy::init()
{
   UInt64 numscripts = Sim()->getCfg()->getInt("hooks/numscripts");
   for(UInt64 i = 0; i < numscripts; ++i) {
      String scriptname = Sim()->getCfg()->getString(String("hooks/script") + itostr(i) + "name");
      if (scriptname.substr(scriptname.length()-3) == ".py") {
         if (! pyInit) {
            setup();
         }

         String args = Sim()->getCfg()->getString(String("hooks/script") + itostr(i) + "args");
         char *argv[] = { (char*)(scriptname.c_str()), (char*)(args.c_str()) };
         PySys_SetArgvEx(2, argv, 0 /* updatepath */);

         printf("Executing Python script %s\n", scriptname.c_str());
         int s = PyRun_SimpleFileEx(fopen(scriptname.c_str(), "r"), scriptname.c_str(), 1 /* closeit */);
         if (s != 0) {
            PyErr_Print();
            fprintf(stderr, "Cannot open Python script %s\n", scriptname.c_str());
            exit(-1);
         }
      }
   }
}

void HooksPy::setup()
{
   pyInit = true;
   Py_SetPythonHome(strdup((String(getenv("GRAPHITE_ROOT")) + "/python_kit").c_str()));
   Py_InitializeEx(0 /* don't initialize signal handlers */);

   // set up all components
   PyConfig::setup();
   PyStats::setup();
   PyHooks::setup();
   PyDvfs::setup();
   PyBbv::setup();
}

void HooksPy::fini()
{
   if (pyInit)
      Py_Finalize();
}

PyObject * HooksPy::callPythonFunction(PyObject *pFunc, PyObject *pArgs)
{
   PyObject *pResult = PyObject_CallObject(pFunc, pArgs);
   Py_XDECREF(pArgs);
   if (pResult == NULL) {
      PyErr_Print();
   }
   return pResult;
}
