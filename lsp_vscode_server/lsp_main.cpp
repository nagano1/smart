#include "lsp_main.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <sstream>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
//#include <emmintrin.h>
#include "../src/LSP_server/LSPMain.hpp"


int main(int argc, char **argv) {
    LSPManager::LSP_main();
    return 0;
}
