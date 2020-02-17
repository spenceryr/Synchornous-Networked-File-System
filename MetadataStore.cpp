#include <iostream>
#include "MetadataStore.hpp"

using namespace std;

MetadataStore::MetadataStore()
{
	cout << "MDS created!" << endl;
}

MetadataStore::~MetadataStore()
{
	cout << "MDS destroyed!" << endl;
}

void MetadataStore::update_meta(string filename, int version, std::vector<std::string> hashes)
{
	(this->metadata_store)[filename] = make_tuple(version, hashes);
}

int MetadataStore::get_version(string filename)
{
	return get<0>((this->metadata_store)[filename]);
}

std::vector<std::string> MetadataStore::get_hashes(string filename)
{
	return get<1>((this->metadata_store[filename]));
}

map<string, file_info_t>& MetadataStore::get_map()
{
	return this->metadata_store;
}