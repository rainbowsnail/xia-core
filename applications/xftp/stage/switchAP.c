#include "stage_utils.h"

//

#define SCAN_INTERVAL 10
//#define GET_SSID_LIST "iwlist wlx60a44ceca928 scanning"//| grep '\\\"[a-zA-Z _0-9.]*\\\"' -o"
#define GET_SSID_LIST "iwlist wlan0 scanning"//| grep '\\\"[a-zA-Z _0-9.]*\\\"' -o"
//#define 

int main(){
	string ssid_list;// = execSystem(GET_SSID_LIST);
	if (ssid_list.empty()) {
// cerr<<"No network\n";
		while (1) {			
			ssid_list = execSystem(GETSSID_CMD);
			printf("ssid list:\n %s ssid_list", ssid_list);
			if (!ssid_list.empty()) {
// cerr<<"Network back\n";
				break;
			}
			usleep(SCAN_INTERVAL * 1000);
		}
	}
	return 0;
}
