; This file is generated using `MCCS_BACKENDS=jbuilder build @settests --auto-promote`

(rule
  (with-stdout-to test-glpk.result (run %{exe:mccs_test.exe} %{dep:test.cudf})))

(rule
  (with-stdout-to test.glpk.reference (cat %{dep:test.output})))

(alias
 (name runtest)
 (action (system "diff %{dep:test-glpk.result} %{dep:test.glpk.reference}")))
