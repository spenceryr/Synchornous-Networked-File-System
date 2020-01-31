#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

#include <XmlRpcCpp.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "SurfStoreProxy.hpp"

int
main(int argc, char *argv[]) {
	auto err_logger = spdlog::stderr_color_mt("stderr");
	spdlog::set_level(spdlog::level::debug);
	spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [thread %t] %v");
	spdlog::get("stderr")->info("Starting RPC client");

	if (argc != 4) {
		cerr << "Usage: " << argv[0] << " server:port /basedir blocksize" << endl;
		exit(1);
	}

	// You should read in the command line arguments here

    // Start up our RPC library.
    XmlRpcClient::Initialize("SurfStoreClient", "0.1");

    try {
		// A hard-coded server, just for testing. You should change
		// this to the argument provided via the command-line
		XmlRpcClient server ("http://localhost:8080/RPC2");
		SurfStoreProxy ss(server);
        
		// Test the 'ping' call
		bool ret1 = ss.ping();
		spdlog::get("stderr")->info("Ping() successful: {}", ret1);

		// Create a block to use for testing
		unsigned char block[16];
		for (unsigned int i = 0; i < 16; i++) {
			block[i] = i % 16;
		}

		// Test the putblock call
		bool ret2 = ss.putblock(XmlRpcValue::makeBase64(block, 16));
		spdlog::get("stderr")->info("Putblock() successful: {}", ret2);

		// Test the getblock call
		XmlRpcValue val = ss.getblock("h0");
		const unsigned char * blockData;
		size_t blockLength;
		val.getBase64(blockData, blockLength);
		spdlog::get("stderr")->info("GetBlock() successfully returned a block of size {}", blockLength);

    } catch (XmlRpcFault & e) {
        cerr << "Client threw error code: " << e.getFaultCode() <<  endl;
        cerr << "Client threw error: " << e.getFaultString() <<  endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }

	XmlRpcClient::Terminate();
    return 0;
}
