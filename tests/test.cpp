#include "test.h"

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
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
#include <emmintrin.h>


#include "gtest/gtest.h"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	
}
