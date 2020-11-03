echo "building Wasm ";
make --version;
clang++-9 --target=wasm32-wasi --sysroot wasi-sysroot -o ws.o -c wasm.cpp;
wasm-ld-9 --help;
wasm-ld-9  --strip-all  -o result.wasm ws.o wasi-sysroot/lib/wasm32-wasi/libc.a wasi-sysroot/lib/wasm32-wasi/libc++.a wasi-sysroot/lib/wasm32-wasi/libc++abi.a;
ls -l;