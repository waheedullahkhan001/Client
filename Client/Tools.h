#pragma once

void UDF_WSAStartup(int *iWsaStartup)
{
	iWsaStartup = WSAStartup(MAKEWORD(2, 2), &Winsockdata);
	if (iWsaStartup != 0) cout << "WSAStartUp Failed!" << endl;
	else cout << "WSAStartUp Success!" << endl;
}


