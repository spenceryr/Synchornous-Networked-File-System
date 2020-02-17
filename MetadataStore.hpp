#ifndef METADATASTORE_HPP
#define METADATASTORE_HPP

#include <vector>
#include <map>
#include <string>

using namespace std;

typedef tuple<int, vector<string>> file_info_t;

class MetadataStore {
public:
	MetadataStore();
	~MetadataStore();

	void update_meta(string filename, int version, vector<string> hashes);

	int get_version(string filename);

	vector<string> get_hashes(string filename);

	map<string, file_info_t> &get_map();

protected:
	map<string, file_info_t> metadata_store;
};

#endif // METADATASTORE_HPP
