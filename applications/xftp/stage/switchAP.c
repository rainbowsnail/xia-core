#include "stage_utils.h"

//

#define SCAN_INTERVAL 10
//#define GET_SSID_LIST "iwlist wlx60a44ceca928 scanning"//| grep '\\\"[a-zA-Z _0-9.]*\\\"' -o"
#define GET_SSID_LIST "iwlist wlx60a44ceca928 scannin | grep '\\\"[a-zA-Z _0-9.]*\\\"' -o"
//#define 

int main(){
	string ssid_list;// = execSystem(GET_SSID_LIST);
	if (ssid_list.empty()) {
// cerr<<"No network\n";
		while (1) {			
			ssid_list = execSystem(GET_SSID_LIST);
			printf("ssid size %d\n",ssid_list.size());
			printf("ssid list:\n %s \n", ssid_list.c_str());
			if (!ssid_list.empty()) {
// cerr<<"Network back\n";
				break;
			}
			usleep(SCAN_INTERVAL * 1000);
		}
	}
	return 0;
}
