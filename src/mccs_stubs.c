#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include "mccs_stubs_cpp.h"

value set_verbosity(value v) {
  return cpp_set_verbosity(v);
}

value gen_problem(value preamble) {
  return cpp_gen_problem(preamble);
}

value add_package_to_problem(value problem, value package) {
  return cpp_add_package_to_problem(problem, package);
}

value set_problem_request(value problem, value request) {
  return cpp_set_problem_request(problem, request);
}

value call_solver(value criteria, value problem) {
  return cpp_call_solver(criteria, problem);
}
