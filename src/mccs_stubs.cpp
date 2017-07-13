#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>

int call_mccs(int argc, char *argv[]);

extern "C" value call_solver(value args)
{
  CAMLparam1(args);
  int argc = Wosize_val(args);
  char * argv[argc];
  for (int i=0; i<argc; i++) argv[i] = String_val(Field(args, i));
  call_mccs(argc, argv);
  CAMLreturn (Val_unit);
}
