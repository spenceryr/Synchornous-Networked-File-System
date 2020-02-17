#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <map>

using namespace std;

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "MetadataStore.hpp"

#include "picosha2.h"

map<string, vector<unsigned char>> block_store;

string hash_block(
    vector<unsigned char> block)
{
    return picosha2::hash256_hex_string(block);
}

// A simple ping. Always returns True
class PingMethod : public xmlrpc_c::method {
public:
    PingMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "b:";
        this->_help = "A simple ping method";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {
        
        paramList.verifyEnd(0);

		spdlog::get("stderr")->info("Ping()");

        *retvalP = xmlrpc_c::value_boolean(true);
    }

protected:
	MetadataStore &m;
};

// Gets a block, given a specific hash value
class GetBlockMethod : public xmlrpc_c::method {
public:
    GetBlockMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "6:s";
        this->_help = "Given a hash value, return the associated block";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

		// Read in the hash value
		string const h(paramList.getString(0));
        paramList.verifyEnd(1);

		spdlog::get("stderr")->info("GetBlock({})", h);
        spdlog::get("stderr")->info("Block {} is: {}", h, &(block_store.at(h)[0]));

        *retvalP = xmlrpc_c::value_bytestring(block_store.at(h));
    }

protected:
	MetadataStore &m;
};

// Puts a block
class PutBlockMethod : public xmlrpc_c::method {
public:
    PutBlockMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "b:6";
        this->_help = "Puts a block in the store";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

		// Read in the block
		vector<unsigned char> const b(paramList.getBytestring(0));
        paramList.verifyEnd(1);

		spdlog::get("stderr")->info("PutBlock({})", &(b[0]));

        block_store[hash_block(b)] = b;

        *retvalP = xmlrpc_c::value_boolean(true);
    }

protected:
	MetadataStore &m;
};

// Determines whether the server has any of the blocks provided
// Input: a list of blocks
// Output: a subset of the input list where the server has that block
class HasBlocksMethod : public xmlrpc_c::method {
public:
    HasBlocksMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "A:A";
        this->_help = "Returns the subset of hash values stored in this server";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

		// Read in the block
		vector<xmlrpc_c::value> const hashlist(paramList.getArray(0));
        paramList.verifyEnd(1);

		spdlog::get("stderr")->info("HasBlocks()");

		vector<xmlrpc_c::value> blocks;
        for (auto const &h : hashlist)
            if ((block_store.count(((xmlrpc_c::value_string)h).cvalue())))
                blocks.push_back((xmlrpc_c::value_string)h);

        *retvalP = xmlrpc_c::value_array(blocks);
    }

protected:
	MetadataStore &m;
};

// Retrieves the server's FileInfoMap
// Input: N/A
// Output: A map of filename to (version, hashlist) tuples
class GetFileInfoMapMethod : public xmlrpc_c::method {
public:
    GetFileInfoMapMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "S:";
        this->_help = "Retrieves the server's FileInfoMap";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        paramList.verifyEnd(0);

		spdlog::get("stderr")->info("GetFileInfomap()");

		map<string, xmlrpc_c::value> fileInfoMap;
        vector<xmlrpc_c::value> finfo;
        for (auto const &x : m.get_map())
        {
            finfo.clear();
            finfo.push_back(xmlrpc_c::value_int(get<0>(get<1>(x))));
            for (auto const &s : get<1>(get<1>(x)))
                finfo.push_back(xmlrpc_c::value_string(s));
            fileInfoMap.insert(pair<string, xmlrpc_c::value>(get<0>(x), xmlrpc_c::value_array(finfo)));
        }

		// vector<xmlrpc_c::value> f1info;
		// f1info.push_back(xmlrpc_c::value_int(3)); // version

		// vector<xmlrpc_c::value> f1blocks;
		// f1blocks.push_back(xmlrpc_c::value_string("h0"));
		// f1blocks.push_back(xmlrpc_c::value_string("h1"));
		// f1blocks.push_back(xmlrpc_c::value_string("h2"));

		// f1info.push_back(xmlrpc_c::value_array(f1blocks));

		// pair<string, xmlrpc_c::value> f1entry("file1.txt", xmlrpc_c::value_array(f1info));
		// fileInfoMap.insert(f1entry);

        *retvalP = xmlrpc_c::value_struct(fileInfoMap);
    }

protected:
	MetadataStore &m;
};

// Updates the entry for a given file
// Input: filename, version, and hashlist
// Output: A boolean indicating success + cloud's current version
class UpdateFileMethod : public xmlrpc_c::method {
public:
    UpdateFileMethod(MetadataStore & mref) : m(mref) {
        this->_signature = "A:siA";
        this->_help = "Updates the FileInfo entry of the given file";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

		// Filename
		string const filename(paramList.getString(0));

		// Version
		int const version(paramList.getInt(1));

		// Updated hashlist
		vector<xmlrpc_c::value> const hashlist(paramList.getArray(2));

        paramList.verifyEnd(3);

		spdlog::get("stderr")->info("UpdateFile({})", filename);

        vector<string> hashes;
        for (auto const &h : hashlist)
            hashes.push_back(((xmlrpc_c::value_string)h).cvalue());

		vector<xmlrpc_c::value> result;
        bool result_bool;
        int result_int;

        if (!(m.get_map().count(filename)))
        {
            if (version == 1)
            {
                m.get_map()[filename] = make_tuple(version, hashes);
                result_bool = true;
                result_int = version;
            }
            else
            {
                result_bool = false;
                result_int = 0;
            }
        }
        else
        {
            if (get<0>(m.get_map()[filename]) == version - 1)
            {
                m.get_map()[filename] = make_tuple(version, hashes);
                result_bool = true;
                result_int = version;
            }
            else
            {
                result_bool = false;
                result_int = get<0>(m.get_map()[filename]);
            }
        }

		result.push_back(xmlrpc_c::value_boolean(result_bool)); // status
		result.push_back(xmlrpc_c::value_int(result_int)); // version

        *retvalP = xmlrpc_c::value_array(result);
    }

protected:
	MetadataStore &m;
};

int 
main(int, char **)
{
	MetadataStore m;
	auto err_logger = spdlog::stderr_color_mt("stderr");
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [thread %t] %v");

	spdlog::get("stderr")->info("Starting RPC server");

    try {
        xmlrpc_c::registry myRegistry;

        xmlrpc_c::methodPtr const PingMethodP(new PingMethod(m));
        myRegistry.addMethod("surfstore.ping", PingMethodP);

        xmlrpc_c::methodPtr const GetBlockMethodP(new GetBlockMethod(m));
        myRegistry.addMethod("surfstore.getblock", GetBlockMethodP);

        xmlrpc_c::methodPtr const PutBlockMethodP(new PutBlockMethod(m));
        myRegistry.addMethod("surfstore.putblock", PutBlockMethodP);

        xmlrpc_c::methodPtr const HasBlocksMethodP(new HasBlocksMethod(m));
        myRegistry.addMethod("surfstore.hasblocks", HasBlocksMethodP);

        xmlrpc_c::methodPtr const GetFileInfoMapMethodP(new GetFileInfoMapMethod(m));
        myRegistry.addMethod("surfstore.getfileinfomap", GetFileInfoMapMethodP);

        xmlrpc_c::methodPtr const UpdateFileMethodP(new UpdateFileMethod(m));
        myRegistry.addMethod("surfstore.updatefile", UpdateFileMethodP);

        xmlrpc_c::serverAbyss myAbyssServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(8092));
        
        myAbyssServer.run();
        // xmlrpc_c::serverAbyss.run() never returns
        assert(false);
    } catch (exception const& e) {
        cerr << "Something failed.  " << e.what() << endl;
    }

    return 0;
}

