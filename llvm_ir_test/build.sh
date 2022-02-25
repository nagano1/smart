set -e
# 
# forgrt 
# 

#clang-10 -S -emit-llvm main.cpp 
#clang-10 -S -emit-llvm lib.cpp
COMPILER=clang++-11
LINKER=lld++-11

$COMPILER -S -emit-llvm -O3 main.cpp
$COMPILER -S -emit-llvm -O3 lib.cpp

llc-11 main.ll
llc-11 lib.ll

#-nostdlib
$LINKER -lpthread  main.s lib.s -o hello && echo start && ./hello


#git diff > diff.patch
git diff --no-prefix -U1000 > diff.patch
