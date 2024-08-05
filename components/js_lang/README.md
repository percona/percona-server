About this component
====================

This component adds to Percona Server for MySQL support of Stored Functions
and Procedures written in JS language or, more formally, ECMAScript by
employing Google's V8 engine.

JS is an abbreviation for JavaScript, but the latter is a trademark of Oracle.

Support for JS language is provided by implementing External Program Execution
service. OTOH the component uses MySQL Stored Program family of services as
well as some other exposed by SQL core to achieve this.


Building and installing
=======================

To enable support for JS Stored Programs in Percona Server for MySQL using
this component one needs to configure the server project using `cmake` with
the following additional flags:

    -DWITH_JS_LANG=ON \
    -DV8_INCLUDE_DIR=<path_to_v8>/include \
    -DV8_LIB_DIR=<path_to_directory_with_libv8monolith_a_library>

and then build the server as usual.

After installing and starting-up the built server one needs to install the
component using:

    INSTALL COMPONENT 'file://component_js_lang';

One also needs to grant new global dynamic `CREATE_JS_ROUTINE` privilege after
that to users who will be creating JS routines (in addition to standard
`CREATE ROUTINE` privilege). For example:

    GRANT CREATE_JS_ROUTINE ON *.* TO root@localhost;


Recommended V8 version and build process for it
-----------------------------------------------

This component was developed and tested with 12.9.202.22 version of V8,
newer versions might not work. Some older versions are known not to work.

We use static, monolith version of V8 library in our component, which is
configured and built using the following commands:

    gn gen "out.gn/static" -vv --fail-on-unused-args \
                           --args='v8_monolithic=true
                                   v8_static_library=true
                                   v8_enable_sandbox=false
                                   v8_enable_pointer_compression=false
                                   is_clang=false
                                   is_asan=false
                                   is_debug=false
                                   is_official_build=false
                                   treat_warnings_as_errors=false
                                   v8_enable_i18n_support=false
                                   v8_use_external_startup_data=false
                                   use_custom_libcxx=false
                                   v8_enable_maglev=false
                                   use_sysroot=false
                                   is_component_build=false'
    ninja -C out.gn/static

Note that using these non-default options solves potential problems with
compatibility of our component with V8 library version built using default
V8 build process, such as usage of clang compiler (moreover own version of
it), custom libcxx and so on.

It is good idea to use build steps (taking into account different V8 version
used and minimal changes to build options) from the `build_v8.sh` script from
Percona's https://github.com/percona/mysql-shell-packaging/ project to prepare
version of V8 library which is compatible with this component.


Examples
========

Some examples of routines in JS language implemented by this component are
available in `component_js_lang` test suite and `js_lang_basic.test`
specifically.


TODO/Future work:
=================

Short-term
----------

- Better error reporting (stacktraces!)
- Console support
- Handling of KILL/timeouts.
- Memory tracking/limiting?
- Handling of async JS features
- Execution of SQL

Long-term
---------

- Observability improvements?
- Debugging?
- Environment support in general?
- Modules support?
- Function attributes and optional behavior (see TODOs)?
- Custom classes for some of types (see TODOs)?

License information:
====================

This component relies on Google V8 engine to execute JS code.
We include V8 headers as well as link with V8 library(ies).

The below you can find the license information for Google V8 engine
(version 12.9.202.22):

V8 License:
-----------

This license applies to all parts of V8 that are not externally
maintained libraries.  The externally maintained libraries used by V8
are:

  - PCRE test suite, located in
    test/mjsunit/third_party/regexp-pcre/regexp-pcre.js.  This is based on the
    test suite from PCRE-7.3, which is copyrighted by the University
    of Cambridge and Google, Inc.  The copyright notice and license
    are embedded in regexp-pcre.js.

  - Layout tests, located in test/mjsunit/third_party/object-keys.  These are
    based on layout tests from webkit.org which are copyrighted by
    Apple Computer, Inc. and released under a 3-clause BSD license.

  - Strongtalk assembler, the basis of the files assembler-arm-inl.h,
    assembler-arm.cc, assembler-arm.h, assembler-ia32-inl.h,
    assembler-ia32.cc, assembler-ia32.h, assembler-x64-inl.h,
    assembler-x64.cc, assembler-x64.h, assembler.cc and assembler.h.
    This code is copyrighted by Sun Microsystems Inc. and released
    under a 3-clause BSD license.

  - Valgrind client API header, located at src/third_party/valgrind/valgrind.h
    This is released under the BSD license.

  - The Wasm C/C++ API headers, located at third_party/wasm-api/wasm.{h,hh}
    This is released under the Apache license. The API's upstream prototype
    implementation also formed the basis of V8's implementation in
    src/wasm/c-api.cc.

These libraries have their own licenses; we recommend you read them,
as their terms may differ from the terms below.

Further license information can be found in LICENSE files located in
sub-directories.

Copyright 2014, the V8 project authors. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Google Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
