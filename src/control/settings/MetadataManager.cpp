#include "MetadataManager.h"
#include "cfg.h"
#include "util/Util.h"

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include <iostream>
using namespace std;

#define REFRESH_SEC 20

MetadataManager::MetadataManager()
{
	XOJ_INIT_TYPE(MetadataManager);

	this->config = NULL;
	this->timer = NULL;
	this->thread = NULL;
}

MetadataManager::~MetadataManager()
{
	XOJ_CHECK_TYPE(MetadataManager);

	io.stop();
	
	delete this->timer;
	this->timer = NULL;
	
	delete this->thread;
	this->thread = NULL;
	
	delete this->config;
	this->config = NULL;

	XOJ_RELEASE_TYPE(MetadataManager);
}

void MetadataManager::setInt(path p, string name, int value)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (p.empty())
	{
		return;
	}

	loadConfigFile();
	checkPath(p);
	
	try
	{
		config->get_child(getINIpathURI(p)).put(name, value);
	}
	catch (exception& e)
	{
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

void MetadataManager::setDouble(path p, string name, double value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty())
	{
		return;
	}
	loadConfigFile();
	checkPath(p);

	try
	{
		config->get_child(getINIpathURI(p)).put(name, value);
	}
	catch (exception& e)
	{
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

void MetadataManager::setString(path p, string name, string value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty())
	{
		return;
	}
	loadConfigFile();
	checkPath(p);

	try
	{
		config->get_child(getINIpathURI(p)).put(name, value);
	}
	catch (exception& e)
	{
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

bool MetadataManager::checkPath(path p)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (p.empty())
	{
		return false;
	}

	loadConfigFile();
	
	try
	{
		config->get_child(getINIpathURI(p));
		return true;
	}
	catch (exception& e)
	{
		config->add_child(getINIpathURI(p), bp::ptree());
		return false;
	}
}

void MetadataManager::updateAccessTime(path p)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (p.empty())
	{
		return;
	}
	loadConfigFile();
	checkPath(p);

	try
	{
		config->get_child(getINIpathURI(p)).put("atime", time(NULL));
	}
	catch (exception& e)
	{
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}
	
	if (this->timer) return;
	
	this->timer = new basio::deadline_timer(io, boost::posix_time::seconds(REFRESH_SEC));
	this->timer->async_wait(boost::bind(save, this, timer));
	this->thread = new boost::thread(boost::bind(&basio::io_service::run, &this->io));
}

void MetadataManager::cleanupMetadata()
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	loadConfigFile();
	
	vector<pair<string, int>> elements;
	
	for (bp::ptree::value_type p : *config)
	{
		if (bf::exists(path(p.first.substr(7))))
		{
			try
			{
				elements.push_back(pair<string, int>(p.first,
						config->get_child(getINIpath(p.first)).get<int>("atime")));
			}
			catch (exception& e)
			{
				cout << bl::format("INI exception: {1}") % e.what() << endl;
			}
		}
	}
	
	std::sort(elements.begin(), elements.end(),
		[](const pair<string, int> &left, const pair<string, int> &right) {
			return left.second >= right.second;
		});
	
	while (elements.size() > METADATA_MAX_ITEMS)
	{
		elements.pop_back();
	}

	bp::ptree* tmpTree = new bp::ptree();
	for (pair<string, int> p : elements)
	{
		tmpTree->add_child(getINIpath(p.first), config->get_child(getINIpath(p.first)));
	}
	
	delete config;
	config = tmpTree;
	tmpTree = NULL;
}

void MetadataManager::copy(path source, path target)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (source.empty() || target.empty())
	{
		return;
	}
	
	try
	{
		config->put_child(getINIpathURI(target), config->get_child(getINIpathURI(source)));
	}
	catch (exception& e)
	{
		cout << bl::format("Cannot copy metadata \"{1}\" to \"{2}\": {3}")
				% source.string() % target.string() % e.what() << endl;
	}
}

bool MetadataManager::save()
{
	XOJ_CHECK_TYPE(MetadataManager);

	this->cleanupMetadata();
	
	try
	{
		bp::ini_parser::write_ini(getFilePath().string(), *this->config);
		return true;
	}
	catch (bp::ini_parser_error const& e)
	{
		cout << bl::format("Could not write metadata file: {1} ({2})")
				% getFilePath().string() % e.what() << endl;
		return false;
	}
}

bool MetadataManager::save(MetadataManager* man, basio::deadline_timer* t)
{
	XOJ_CHECK_TYPE_OBJ(man, MetadataManager);
	
	if (t != NULL)
	{
		t->expires_at(t->expires_at() + boost::posix_time::seconds(REFRESH_SEC));
		t->async_wait(boost::bind(save, man, t));
	}
	
	return man->save();
}

void MetadataManager::loadConfigFile()
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (this->config)
	{
		return;
	}
	
	config = new bp::ptree();
	
	path filepath = getFilePath();
	if (bf::exists(filepath))
	{
		try
		{
			bp::ini_parser::read_ini(filepath.string(), *config);
		}
		catch (bp::ini_parser_error const& e)
		{
			cout << bl::format("Metadata file \"{1}\" is invalid: {2}")
					% filepath.string() % e.what() << endl;
		}
	}
}

bool MetadataManager::getInt(path p, string name, int& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty())
	{
		return false;
	}

	loadConfigFile();

	try
	{
		value = config->get_child(getINIpathURI(p)).get<int>(name);
		return true;
	}
	catch (std::exception const& e)
	{
		return false;
	}
}

bool MetadataManager::getDouble(path p, string name, double& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty())
	{
		return false;
	}
	loadConfigFile();

	try
	{
		value = config->get_child(getINIpathURI(p)).get<int>(name);
		return true;
	}
	catch (std::exception const& e)
	{
		return false;
	}
}

bool MetadataManager::getString(path p, string name, string& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty())
	{
		return false;
	}

	loadConfigFile();

	try
	{
		value = config->get_child(getINIpathURI(p)).get<string>(name);
		return true;
	}
	catch (std::exception const& e)
	{
		return false;
	}
}

path MetadataManager::getFilePath()
{
	return Util::getSettingsFile(METADATA_FILE);
}

//kinda workaround for now – it probably wouldn't work on Windows
bp::ptree::path_type MetadataManager::getINIpathURI(path p)
{
#ifdef _WIN32
	string spath = p.string();
	StringUtils::replace_all_chars(spath, {replace_pair('\\', "/")});
	return getINIpath(CONCAT("file://", spath));
#else
	return getINIpath(CONCAT("file://", p.string()));
#endif
}


bp::ptree::path_type MetadataManager::getINIpath(string s)
{
	return bp::ptree::path_type(s, '\n');
}
