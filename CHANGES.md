1.1+18 (2024/08/21)
-------------------

* Adding missing dependency on `conf-c++` (#52 @dra27)

* Disable every warnings when compiling the vendored `glpk` code (#41 @kit-ty-kate @MisterDA)

* Replace `sprintf` and `vsprintf` with `snprintf` and `vsnprintf` (#41 @MisterDA)

* Always compile C++ files with `-x c++` on non-msvc compilers to avoid issues when using `clang` (#53 @kit-ty-kate)
