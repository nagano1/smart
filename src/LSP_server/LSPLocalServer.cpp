#define _CRT_SECURE_NO_WARNINGS


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

#include "./httplib.h"
//#include <emmintrin.h>

#include "./LSPLocalServer.hpp"

#include "common.hpp"


void LSPHttpServer::close() {
	//svr.stop();
}

void LSPHttpServer::LSP_server() {
	// https://github.com/yhirose/cpp-httplib
	
	httplib::Server svr;

	svr.Get("/hi", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content("<div>Hello World!</div>", "text/html");
	});

	svr.listen("0.0.0.0", 8080);
}