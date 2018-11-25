#include <config_file_reader.h>
#include <types.h>

//#include <cstdio>

#ifdef _MSC_VER
#define	snprintf	sprintf_s
#endif

using namespace muduo;

ConfigFileReader::ConfigFileReader(const char* filename)
{
	_LoadFile(filename);
}

ConfigFileReader::~ConfigFileReader()
{
}

char* ConfigFileReader::GetConfigName(const char* name)
{
	if (!m_load_ok)
		return NULL;

	char* value = NULL;
	std::map<std::string, std::string>::iterator it = m_config_map.find(name);
	if (it != m_config_map.end()) {
		value = (char*)it->second.c_str();
	}

	return value;
}

int ConfigFileReader::SetConfigValue(const char* name, const char* value)
{
	if (!m_load_ok)
		return -1;

	std::map<std::string, std::string>::iterator it = m_config_map.find(name);
	if (it != m_config_map.end())
	{
		it->second = value;
	}
	else
	{
		m_config_map.insert(std::make_pair(name, value));
	}
	return _WriteFile();
}
void ConfigFileReader::_LoadFile(const char* filename)
{
	m_config_file.clear();
	m_config_file.append(filename);
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		//log("can not open %s,errno = %d", filename, errno);
		return;
	}

	char buf[256];
	for (;;)
	{
		char* p = fgets(buf, 256, fp);
		if (!p)
			break;

		size_t len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;			// remove \n at the end

		char* ch = strchr(buf, '#');	// remove string start with #
		if (ch)
			*ch = 0;

		if (strlen(buf) == 0)
			continue;

		_ParseLine(buf);
	}

	fclose(fp);
	m_load_ok = true;
}

int ConfigFileReader::_WriteFile(const char* filename)
{
	FILE* fp = NULL;
	if (filename == NULL)
	{
		fp = fopen(m_config_file.c_str(), "w");
	}
	else
	{
		fp = fopen(filename, "w");
	}
	if (fp == NULL)
	{
		return -1;
	}

	char szPaire[128];
	std::map<std::string, std::string>::iterator it = m_config_map.begin();
	for (; it != m_config_map.end(); it++)
	{
		memset(szPaire, 0, sizeof(szPaire));
		snprintf(szPaire, sizeof(szPaire), "%s=%s\n", it->first.c_str(), it->second.c_str());
		uint32_t ret = fwrite(szPaire, strlen(szPaire), 1, fp);
		if (ret != 1)
		{
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
	return 0;
}
void ConfigFileReader::_ParseLine(char* line)
{
	char* p = strchr(line, '=');
	if (p == NULL)
		return;

	*p = 0;
	char* key = _TrimSpace(line);
	char* value = _TrimSpace(p + 1);
	if (key && value)
	{
		m_config_map.insert(std::make_pair(key, value));
	}
}

char* ConfigFileReader::_TrimSpace(char* name)
{
	// remove starting space or tab
	char* start_pos = name;
	while ((*start_pos == ' ') || (*start_pos == '\t'))
	{
		start_pos++;
	}

	if (strlen(start_pos) == 0)
		return NULL;

	// remove ending space or tab
	char* end_pos = name + strlen(name) - 1;
	while ((*end_pos == ' ') || (*end_pos == '\t'))
	{
		*end_pos = 0;
		end_pos--;
	}

	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0)
		return NULL;

	return start_pos;
}
