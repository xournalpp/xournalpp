#include "MetadataManager.h"
#include "../../cfg.h"
#include "../../util/Util.h"

#include <gtk/gtk.h>

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include <iostream>
using namespace std;

MetadataManager::MetadataManager()
{
	XOJ_INIT_TYPE(MetadataManager);

	this->config = NULL;
	this->timeoutId = 0;
}

MetadataManager::~MetadataManager()
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (this->timeoutId)
	{
		g_source_remove(this->timeoutId);
		this->timeoutId = 0;
		save(this);
	}

	if (this->config) delete this->config;

	XOJ_RELEASE_TYPE(MetadataManager);
}

void MetadataManager::setInt(path p, string name, int value)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (p.empty()) return;
	loadConfigFile();
	
	try {
		config->get_child(getURI(p)).put(name, value);
	} catch (exception& e) {
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

void MetadataManager::setDouble(path p, string name, double value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty()) return;
	loadConfigFile();

	try {
		config->get_child(getURI(p)).put(name, value);
	} catch (exception& e) {
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

void MetadataManager::setString(path p, string name, string value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty()) return;
	loadConfigFile();

	try {
		config->get_child(getURI(p)).put(name, value);
	} catch (exception& e) {
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}

	updateAccessTime(p);
}

void MetadataManager::openFile()
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (!file.is_open())
	{
		//file.open();
	}
}

void MetadataManager::updateAccessTime(path p)
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (p.empty()) return;
	loadConfigFile();

	try {
		config->get_child(getURI(p)).put("atime", time(NULL));
	} catch (exception& e) {
		cout << bl::format("INI exception: {1}") % e.what() << endl;
	}
	
	if (this->timeoutId) return;

	this->timeoutId = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE, 2,
												 (GSourceFunc) save, this, NULL);
}

void MetadataManager::cleanupMetadata()
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	loadConfigFile();
	
	vector<pair<string, int>> elements;
	
	for (bp::ptree::value_type p : config->get_child(""))
	{
		if (bf::exists(path(p.first.substr(7))))
		{
			elements.push_back(pair<string, int>(p.first,
					config->get_child(p.first).get<int>(".atime")));
		}
	}
	
	std::sort(elements.begin(), elements.end(),
		[](const pair<string, int> &left, const pair<string, int> &right) {
			return left.second < right.second;
		});
	
	while (elements.size() > METADATA_MAX_ITEMS)
	{
		elements.pop_back();
	}

	bp::ptree* tmpTree = new bp::ptree();
	for (pair<string, int> p : elements)
	{
		tmpTree->add_child(p.first, config->get_child(p.first));
	}
	
	delete config;
	config = tmpTree;
	tmpTree = NULL;
}

void MetadataManager::move(path source, path target)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (source.empty() || target.empty()) return;
	
	for (bp::ptree::value_type p : config->get_child(source.string()))
	{
		try {
			config->get_child(getURI(target)).put_child(p.first, p.second);
			config->erase(getURI(source));
		} catch (exception& e) {
			cout << bl::format("Cannot move metadata \"{1}\" to \"{2}\": {3}")
					% source.string() % target.string() % e.what() << endl;
		}
	}
}

bool MetadataManager::save(MetadataManager* manager)
{
	XOJ_CHECK_TYPE_OBJ(manager, MetadataManager);

	manager->timeoutId = 0;

	manager->cleanupMetadata();
	
	try {
		bp::ini_parser::write_ini(getFilePath().string(), *manager->config);
		return true;
	} catch (bp::ini_parser_error const& e) {
		cout << bl::format("Could not write metadata file: {1} ({2})")
				% getFilePath().string() % e.what() << endl;
		return false;
	}
}

void MetadataManager::loadConfigFile()
{
	XOJ_CHECK_TYPE(MetadataManager);
	
	if (this->config) return;
	
	config = new bp::ptree();
	
	path filepath = getFilePath();
	if (bf::exists(filepath))
	{
		try {
			bp::ini_parser::read_ini(filepath.string(), *config);
		} catch (bp::ini_parser_error const& e) {
			cout << bl::format("Metadata file \"{1}\" is invalid: {2}")
					% filepath.string() % e.what() << endl;
		}
	}
}

bool MetadataManager::getInt(path p, string name, int& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty()) return false;
	loadConfigFile();

	try {
		int v = config->get_child(getURI(p)).get<int>(name);
		value = v;
		return true;
	} catch (std::exception const& e) {
		return false;
	}
}

bool MetadataManager::getDouble(path p, string name, double& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty()) return false;
	loadConfigFile();

	try {
		double v = config->get_child(getURI(p)).get<int>(name);
		value = v;
		return true;
	} catch (std::exception const& e) {
		return false;
	}
}

bool MetadataManager::getString(path p, string name, string& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (p.empty()) return false;
	loadConfigFile();

	try {
		string v = config->get_child(getURI(p)).get<string>(name);
		value = v;
		return true;
	} catch (std::exception const& e) {
		return false;
	}
}

path MetadataManager::getFilePath() {
	return Util::getSettingsFile(METADATA_FILE);
}

string MetadataManager::getURI(path &p)
{
	return CONCAT("file://", p.string());
}
