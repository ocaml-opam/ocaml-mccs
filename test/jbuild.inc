; This file is generated using `MCCS_BACKENDS=jbuilder build @settests --auto-promote`

(rule
  (with-stdout-to test-glpk.result (run ${exe:mccs_test.exe} ${path:test.cudf})))

(rule
  (with-stdout-to test.glpk.reference (cat ${path:test.output})))

(alias
 ((name runtest)
  (action (system "diff ${path:test-glpk.result} ${path:test.glpk.reference}"))))
