#include <config_file_reader.h>

using namespace muduo;

int main2()
{
	//E:\muduo\src\build\muduo\Debug\muduo.exe
	ConfigFileReader config_file("E:\\muduo\\src\\build\\muduo\\Debug\\msgserver.conf");

	char* listen_ip = config_file.GetConfigName("ListenIP");
	char* str_listen_port = config_file.GetConfigName("ListenPort");
	char* ip_addr1 = config_file.GetConfigName("IpAddr1");	
	char* ip_addr2 = config_file.GetConfigName("IpAddr2");	
	char* str_max_conn_cnt = config_file.GetConfigName("MaxConnCnt");
	char* str_aes_key = config_file.GetConfigName("aesKey");
	return 0;
}