#include <iostream>
#include "MetadataStore.hpp"

using namespace std;

MetadataStore::MetadataStore()
	: num(0)
{
	cout << "MDS created!" << endl;
}

MetadataStore::~MetadataStore()
{
	cout << "MDS destroyed!" << endl;
}

void MetadataStore::go()
{
	cout << "I've done this " << num << " times!" << endl;
	num += 1;
}
