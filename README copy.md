
![CMake Build Matrix](https://github.com/nagano1/door/workflows/CMake%20Build%20Matrix/badge.svg)

# Top Folders
- **AndroidCanLang** : Android app running unit tests(Google Test)
- **CoreTestApp** : iOS app
- **lib** : contains Google Test
- **llvm_ir_test** : Simulation for generating a final output from new
- **lsp_vscode_server** : Visual Studio Code Extension for providing new lang dev experienc 
- **src** : Main C/C++  source code
- **test_vscode_vsix** : Visual Studio Code Extension for providing new lang dev experienc
- **tests** : Visual Studio Code Extension for providing new lang dev experienc
- **visual_studio_console_sln** : Visual Studio Cosole Project
- **wasm_project** : building Wasm project (contains wasi-sdk-11)
- **wasm_project_test** : npm project folder for `npm install puppeteer` and run a test on Github actions



Deed

# Install on Ubuntu 
```bash
sudo apt-get install llvm-10-dev cmake clang-10 lld-10
export CXX=clang++-10

cd build
cmake --config Release ..
cmake --build . --config Release 
```

# Windows
## command line: cmake  Windows
install cmake and llvm

cd build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
tests\Release\main_tst.exe
```
## for Visual Studio 2019
the sln file is configured for VS2017 currently. Required to install below components to build with VS2019. 
- MSVC v141 - VS 2017 C++ x64/x86 build tools
- Windows 10 SDK (10.0.17134.0)

and you might need to update google test in Nuget manager. 
- Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn



## LINKS
- https://code.visualstudio.com/api/language-extensions/language-server-extension-guide
- https://github.com/microsoft/vscode-languageserver-node
- https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/

- https://docs.github.com/en/free-pro-team@latest/rest/overview/api-previews

- https://stackoverflow.com/questions/11031244/jgit-validate-if-repository-is-valid
- https://github.com/wasm3/wasm3/tree/master/platforms/ios
- https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md

- https://github.com/google/cpu_features
- https://denspe.blog.fc2.com/blog-entry-174.html
- https://denspe.blog.fc2.com/blog-entry-173.html

## llvm IR commands
```bash
sudo apt-get install libc6-dev-i386
sudo apt-get install gcc-multilib g++-multilib

clang-10 -S -emit-llvm -O3 main.c && llc-10 main.ll && clang-10 main.s -o hello
```

<aowfeijf ara:=awfjo-ajweoifjoaw aowiejf:=jofaiwe-ojawiefwao awfea:ajwoipjfpowa afe="jafowief" awf=foiawjeopaw


{
    jofiawe:"jofwiaj"
    awef=true
    awef:"fjioaw-awjeoifa-ajwoefjo"
    awef=32414
    awef23:awoiefja
    joiwaep = [
        {} [] {}
    ]
}

<(i64)>32
<(i32)>
<(float)>
<(int)>
<int>


auto clangPath = when {
    fjaowieee ->
    fjioawejfoawje -> 
    else -> 
} also {
    it * 3;
} also {
    ret it / 2;
}


var a = (

);

func(24, lmd (x,k) -> {return x*k;})


/* Circular references are not allowed by default */
class A {
    B* valB?;
}

class B {
    C* valC = new C(2342);
}

class C {
    A* valA?;
}


Deed
/* Every property which is possibly circulated needs to be marked as circ(Circular) to execute/compile */
class A {
    circ B* valB? = nullptr; null;

    ret fjioaeee;
}

class B {
    circ C* valC = new C(2342);
}

class C {
    circ A* valA?;
}



/* here below is an example of non circulated */
class A {
    int g = 4321;



    ctor(int a) {
        // def has to be at end of the ctor
        public def (
            auto* tempC = new C(val);
        )

        private def (

        )
    }

    ctor(float b) {
        this(a:3.4f)
    }
}

class B {
    A* valA?;
}

class C {
    A* valA?;
}

fn func(circ B* b) {

}