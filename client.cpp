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
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [thread %t] %v");
	spdlog::get("stderr")->info("Starting RPC client");

	if (argc != 4) {
		cerr << "Usage: " << argv[0] << " server:port /basedir blocksize" << endl;
		exit(1);
	}

	// You should read in the command line arguments here

	string server_port = argv[1];
	string base_dir = argv[2];
	int blocksize = stoi(argv[3]);

    // Start up our RPC library.
    XmlRpcClient::Initialize("SurfStoreClient", "0.1");

    try {
		// A hard-coded server, just for testing. You should change
		// this to the argument provided via the command-line
		server_port = "http://" + server_port + "/RPC2";
		XmlRpcClient server (server_port);
		SurfStoreProxy ss(server);

		char *real_path = realpath(base_dir.c_str(), NULL);
		string abs_path (real_path);
		free(real_path);
        
		// Test the 'ping' call
		bool ret1 = ss.ping();
		spdlog::get("stderr")->info("Ping() successful: {}", ret1);

		ss.set_base_dir(abs_path);
		spdlog::get("stderr")->info("Basedir set to: {}", abs_path);
		ss.set_block_size(blocksize);
		spdlog::get("stderr")->info("Blocksize set to: {}", blocksize);
		ss.load_local_files();
		spdlog::get("stderr")->info("Local files loaded");
		ss.load_local_index();
		spdlog::get("stderr")->info("Local index loaded");
		ss.load_remote_index();
		spdlog::get("stderr")->info("Remote index loaded");
		ss.sync();
		spdlog::get("stderr")->info("Sync complete");
		ss.build_indextxt();
		spdlog::get("stderr")->info("Index.txt built");

    } catch (XmlRpcFault & e) {
        cerr << "Client threw error code: " << e.getFaultCode() <<  endl;
        cerr << "Client threw error: " << e.getFaultString() <<  endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }

	XmlRpcClient::Terminate();
    return 0;
}
