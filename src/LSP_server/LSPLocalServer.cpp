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

static char* _text = nullptr;

void LSPHttpServer::LSP_server() {
	// https://github.com/yhirose/cpp-httplib
	
	httplib::Server svr;

	svr.Get("/hi", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content(_text == nullptr ? "fjowi": _text, "text/html");
	});

	svr.Get("/stream", [&](const httplib::Request& req, httplib::Response& res) {
		res.set_content_provider(
			"text/html", // Content type
			[&](size_t offset, httplib::DataSink& sink) {
				int count = 5;

				while (count > 0/* there is still data */) {
					count--;
					Sleep(1000);
					printf("aa");
					// prepare data...
					sink.write("aaa", 3);
				}
				
				printf("end");

				sink.done(); // No more data
				return true; // return 'false' if you want to cancel the process.
			});
		});

	svr.listen("0.0.0.0", 8080);

}

void LSPHttpServer::passText(char* text, int textLen) {
	if (_text != nullptr) {
		free(_text);
	}

	auto* chars = (char*)malloc(textLen + 1000);
	_text = chars;

	sprintf((char*)_text, R"(<span>%s</span>)", text);
}
