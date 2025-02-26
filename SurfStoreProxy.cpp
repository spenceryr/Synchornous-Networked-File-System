// SurfStoreProxy.cpp - xmlrpc-c C++ proxy class
// Auto-generated by xml-rpc-api2cpp.

#include <xmlrpc-c/oldcppwrapper.hpp>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include "SurfStoreProxy.hpp"
#include "picosha2.h"
#include "MyFileSystem.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;

bool SurfStoreProxy::ping () {
    XmlRpcValue params(XmlRpcValue::makeArray());
    XmlRpcValue result(this->mClient.call("surfstore.ping", params));
    return result.getBool();
}

XmlRpcValue /*base64*/ SurfStoreProxy::getblock (std::string const string1) {
    spdlog::get("stderr")->info("Getting block for {}", string1);
    XmlRpcValue params(XmlRpcValue::makeArray());
    params.arrayAppendItem(XmlRpcValue::makeString(string1));
    XmlRpcValue result(this->mClient.call("surfstore.getblock", params));
    return result;
}

bool SurfStoreProxy::putblock (XmlRpcValue /*base64*/ base641) {
    XmlRpcValue params(XmlRpcValue::makeArray());
    params.arrayAppendItem(base641);
    XmlRpcValue result(this->mClient.call("surfstore.putblock", params));
    return result.getBool();
}

XmlRpcValue /*array*/ SurfStoreProxy::hasblocks (XmlRpcValue /*array*/ array1) {
    XmlRpcValue params(XmlRpcValue::makeArray());
    params.arrayAppendItem(array1);
    XmlRpcValue result(this->mClient.call("surfstore.hasblocks", params));
    return result;
}

XmlRpcValue /*struct*/ SurfStoreProxy::getfileinfomap () {
    spdlog::get("stderr")->info("Getting fileinfomap");
    XmlRpcValue params(XmlRpcValue::makeArray());
    XmlRpcValue result(this->mClient.call("surfstore.getfileinfomap", params));
    return result;
}

XmlRpcValue /*array*/ SurfStoreProxy::updatefile (std::string const string1, XmlRpcValue::int32 const int2, XmlRpcValue /*array*/ array3) {
    spdlog::get("stderr")->info("Updating remote file for {}", string1);
    XmlRpcValue params(XmlRpcValue::makeArray());
    params.arrayAppendItem(XmlRpcValue::makeString(string1));
    params.arrayAppendItem(XmlRpcValue::makeInt(int2));
    params.arrayAppendItem(array3);
    XmlRpcValue result(this->mClient.call("surfstore.updatefile", params));
    return result;
}

void SurfStoreProxy::set_base_dir(string dir)
{
    this->base_dir = dir;
}

void SurfStoreProxy::set_block_size(int i)
{
    this->block_size = i;
}

void SurfStoreProxy::load_local_index()
{
    spdlog::get("stderr")->info("Loading local index");
    map<string, file_info_t> result;
    vector<string> hashes;
    string line, line_value, filename;
    int version;
    size_t found_start, found_end;
    ifstream f (build_path(this->base_dir, "index.txt"), ios::in);
    if (!f)
    {
        spdlog::get("stderr")->info("index.txt doesn't exist");
        build_indextxt(true);
        f.open(build_path(this->base_dir, "index.txt"), ios::in);
    }
    while (getline(f, line))
    {
        hashes.clear();
        found_end = line.find(",");
        filename = line.substr(0, found_end);
        found_start = found_end + 1;
        found_end = line.find(",", found_start);
        version = stoi(line.substr(found_start, found_end-found_start));
        found_start = found_end + 1;
        while ((found_end = line.find(" ", found_start)) != string::npos)
        {
            hashes.push_back(line.substr(found_start, found_end-found_start));
            found_start = found_end + 1;
        }
        if (found_start != line.length())
            hashes.push_back(line.substr(found_start));

        result.insert(pair<string, file_info_t>(filename, file_info_t {version, hashes}));
    }
    f.close();
    this->local_index = result;
}

void SurfStoreProxy::load_local_files()
{
    spdlog::get("stderr")->info("Loading local files");
    map<string, vector<string>> result;
    string path_string;
    for (auto &f : get_files_in_dir(this->base_dir))
    {
        spdlog::get("stderr")->info("Local file {} found", f);
        if (f == "index.txt")
            continue;
        path_string = build_path(base_dir, f);
        result.insert(pair<string, vector<string>>(f, load_local_blocks(path_string)));
    }
    this->local_files = result;
}

vector<string> SurfStoreProxy::load_local_blocks(
    string path_string)
{
    spdlog::get("stderr")->info("Loading local blocks");
    vector<string> hashes_v;
    vector<unsigned char> char_v;
    int fsize;
    int i;
    ifstream f (path_string, ifstream::in | ifstream::binary | ifstream::ate);
    fsize = f.tellg();
    unsigned char *char_block = new unsigned char[fsize];
    f.seekg (0, ios::beg);
    f.read((char *)char_block, fsize);
    f.close();
    for (i = 0; i < fsize; i += this->block_size)
    {
        if (i + this->block_size > fsize)
        {
            spdlog::get("stderr")->info("Block found: {}", string(char_block + i, char_block + fsize));
            char_v.assign(char_block + i, char_block + fsize);
        }
        else
        {
            spdlog::get("stderr")->info("Block found: {}", string(char_block + i, char_block + this->block_size + i));
            char_v.assign(char_block + i, char_block + this->block_size + i);
        }
        hashes_v.push_back(hash_block(char_v));
        if (!(this->local_block_store.count(hashes_v.back())))
            this->local_block_store.insert(pair<string, vector<unsigned char>>(hashes_v.back(), char_v));
    }
    delete[] char_block;
    return hashes_v;
}

string SurfStoreProxy::hash_block(
    vector<unsigned char> block)
{
    spdlog::get("stderr")->info("Hash result: {}", picosha2::hash256_hex_string(block));
    return picosha2::hash256_hex_string(block);
}

void SurfStoreProxy::load_remote_index()
{
    spdlog::get("stderr")->info("Loading remote index");
    map<string, file_info_t> result;
    XmlRpcValue xml_map = getfileinfomap().getStruct();
    XmlRpcValue val;
    
    int version;
    string filename;
    vector<string> hashes;
    size_t hash_list_size;
    for (size_t i = 0; i < xml_map.structSize(); i++)
    {
        hashes.clear();
        xml_map.structGetKeyAndValue(i, filename, val);
        val = val.getArray();
        hash_list_size = val.arraySize();

        version = val.arrayGetItem(0).getInt();
        for (size_t j = 1; j < hash_list_size; j++)
        {
            hashes.push_back(val.arrayGetItem(j).getString());
            spdlog::get("stderr")->info("Hash for {} is {}", filename, hashes.back());
        }
        result.insert(pair<string, file_info_t>(filename, make_tuple(version, hashes)));
    }
    this->remote_index = result;
}

file_info_t SurfStoreProxy::redownload_file(
    string filename)
{
    spdlog::get("stderr")->info("Redownloading file {}", filename);
    file_info_t result;
    XmlRpcValue xml_map = (getfileinfomap().getStruct());
    XmlRpcValue val;
    
    int version;
    vector<string> hashes;
    file_info_t index_arr;

    val = xml_map.structGetValue(filename).getArray();

    version = val.arrayGetItem(0).getInt();
    for (size_t j = 1; j < val.arraySize(); j++)
        hashes.push_back(val.arrayGetItem(j).getString());
    return make_tuple(version, hashes);
}

void SurfStoreProxy::update_local_blocks(
    string filename,
    file_info_t remote_file_data)
{
    spdlog::get("stderr")->info("Updating local blocks for {}", filename);
    if (get<1>(remote_file_data) == (vector<string> (1, "0")))
    {
        spdlog::get("stderr")->info("Deleting file {}", filename);
        remove(build_path(this->base_dir, filename).c_str());
        (this->local_index)[filename] = remote_file_data;
        (this->local_files)[filename] = get<1>(remote_file_data);
        return;
    }

    ofstream f (build_path(this->base_dir, filename), ofstream::out | ofstream::binary | ofstream::trunc);
    const unsigned char *out;
    size_t bs;
    XmlRpcValue block;

    for (string &hash : get<1>(remote_file_data))
    {
        if (!(this->local_block_store.count(hash)))
        {
            block = getblock(hash);
            block.getBase64(out, bs);
            spdlog::get("stderr")->info("Downloaded block: {}", string(out, out + bs));
            (this->local_block_store)[hash].assign(out, out+bs);
        }


        f.write((char *)&((this->local_block_store)[hash][0]), (this->local_block_store)[hash].size());
    }
    f.close();

    (this->local_index)[filename] = remote_file_data;
    (this->local_files)[filename] = get<1>(remote_file_data);
}

void SurfStoreProxy::attempt_push(
    string filename)
{
    XmlRpcValue local_hashes_xml, server_hashes_xml;
    unordered_set<string> server_hashes_set;
    int new_version;

    if (!((this->local_index).count(filename)))
        return;

    new_version = get<0>((this->local_index)[filename]) + 1;

    local_hashes_xml = XmlRpcValue::makeArray();
    for (auto const &s : get<1>((this->local_index)[filename]))
        local_hashes_xml.arrayAppendItem(XmlRpcValue::makeString(s));
    
    server_hashes_xml = hasblocks(local_hashes_xml);
    server_hashes_xml = server_hashes_xml.getArray();
    for (size_t j = 1; j < server_hashes_xml.arraySize(); j++)
        server_hashes_set.insert(server_hashes_xml.arrayGetItem(j).getString());

    if (get<1>((this->local_index)[filename]) != (vector<string> (1, "0")))
        for (auto const &hash : get<1>((this->local_index)[filename]))
            if (!(server_hashes_set.count(hash)))
                putblock(XmlRpcValue::makeBase64(
                    &((this->local_block_store)[hash][0]),
                    (this->local_block_store)[hash].size()));

    XmlRpcValue server_version = updatefile(filename, new_version, local_hashes_xml);
    server_version = server_version.getArray();

    if (server_version.arrayGetItem(0).getBool() == false && server_version.arrayGetItem(1).getInt() != new_version)
        update_local_blocks(filename, redownload_file(filename));
    else
        get<0>((this->local_index)[filename]) = new_version;
}

void SurfStoreProxy::sync()
{
    vector<string> hashes;
    for (auto const &f : this->local_files)
    {
        hashes = get<1>(f);
        if (this->local_index.count(get<0>(f)))
            get<1>(this->local_index[get<0>(f)]) = hashes;
        else
            this->local_index[get<0>(f)] = make_tuple(0, hashes);
    }

    for (auto const &f : this->local_index)
    {
        if (!(this->local_files.count(get<0>(f))))
        {
            hashes.assign(1, "0");
            get<1>(this->local_index[get<0>(f)]) = hashes;
        }
    }

    for (auto const &f : this->remote_index)
    {
        if (this->local_index.count(get<0>(f))) // if local index has file from remote index
        {
            if ((get<0>(get<1>(f)) != get<0>(this->local_index[get<0>(f)]))) // if version numbers are different
                update_local_blocks(get<0>(f), get<1>(f));
            else if (get<1>(get<1>(f)) != get<1>(this->local_index[get<0>(f)])) // if version numbers are same but hashes are different
                attempt_push(get<0>(f));
        }
        else
            update_local_blocks(get<0>(f), get<1>(f));
    }

    for (auto const &f : this->local_index)
    {
        if ((this->remote_index).count(get<0>(f)))
            continue;
        attempt_push(get<0>(f));
    }
}

void SurfStoreProxy::build_indextxt(bool from_local)
{
    ofstream ofile (build_path(this->base_dir, "index.txt"), ofstream::out | ofstream::trunc);
    bool space;

    if (from_local)
    {
        for (auto const &f : this->local_files)
        {
            space = false;
            ofile << get<0>(f) << "," << "0" << ",";

            for (auto const &hash : get<1>(f))
            {
                if (space)
                    ofile << " ";
                else
                    space = true;
                ofile << hash;
            }
            ofile << endl;
        }
        ofile.close();
        return;
    }



    for (auto const &f : this->local_index)
    {
        space = false;
        ofile << get<0>(f) << "," << get<0>(get<1>(f)) << ",";

        for (auto const &hash : get<1>(get<1>(f)))
        {
            if (space)
                ofile << " ";
            else
                space = true;
            ofile << hash;
        }
        ofile << endl;
    }
    ofile.close();
}