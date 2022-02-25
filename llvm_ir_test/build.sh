set -e
# 
# forgrt 
# 

#clang-10 -S -emit-llvm main.cpp 
#clang-10 -S -emit-llvm lib.cpp
COMPILER=clang++-10
LINKER=clang++-10

$COMPILER -S -emit-llvm -O3 main.cpp
$COMPILER -S -emit-llvm -O3 lib.cpp

llc-10 main.ll
llc-10 lib.ll

#-nostdlib
$LINKER -lpthread  main.s lib.s -o hello && echo start && ./hello


#git diff > diff.patch
git diff --no-prefix -U1000 > diff.patch
