#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "MyFileSystem.hpp"

using namespace std;

bool is_file(string path)
{
	struct stat s;
	stat(path.c_str(), &s);
	return (bool)(s.st_mode & S_IFREG);
}

string build_path(string base_dir, string filename)
{
	if (base_dir.back() != '/')
		return (base_dir + '/' + filename);
	else
		return (base_dir + filename);
}

vector<string> get_files_in_dir(string base_dir)
{
	vector<string> result;
	struct dirent *de = (struct dirent *)NULL;
	DIR *dp = (DIR *)NULL;
	dp = opendir(base_dir.c_str());
	if (!dp)
		return result;

	while ((de = readdir(dp)))
	{
		if (is_file(build_path(base_dir, de->d_name)))
			result.push_back(string(de->d_name));
	}
	return result;
}
