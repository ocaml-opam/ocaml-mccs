#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>

void install_sigint_handler(void);
void restore_sigint_handler(void);

extern "C" CAMLprim value mccs_test_sigint(value unit) {
  CAMLparam1(unit);

  int ret = 0;

  try {
    install_sigint_handler();
    while (1) _sleep(10);
  } catch (int p) {
    if (p == 130) {
      ret = -2;
    } else {
      ret = -3;
    }
  } catch (...) {
    ret = -1;
  }

  if (ret == -2) {
    caml_raise_constant(*caml_named_value("Sys.Break"));
  }

  CAMLreturn(Val_unit);
}
