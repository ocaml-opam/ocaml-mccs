#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <caml/mlvalues.h>

EXTERNC value cpp_set_verbosity(value v);
EXTERNC value cpp_gen_problem(value preamble);
EXTERNC value cpp_add_package_to_problem(value problem, value package);
EXTERNC value cpp_set_problem_request(value problem, value request);
EXTERNC value cpp_call_solver(value criteria, value problem);
