// KC_Trader_001.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
using namespace std;
HANDLE hMutex = CreateMutex(NULL, false, NULL);
string GetExePath()
{
	char path[MAX_PATH];
	wchar_t path_w[MAX_PATH];
	GetModuleFileName(NULL, path_w, sizeof(path_w));
	(_tcsrchr(path_w, _T('\\')))[1] = 0;
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, path_w, -1, path, MAX_PATH, NULL, NULL);
	string path_str = path;
	return path_str;
}

AccountInfo GetAccountInfo(string account_name)
{
	string path = GetExePath();
	string fn_plus = path + account_name + "/�˺���Ϣ.txt";
	ifstream in_file(fn_plus);
	string s;
	AccountInfo account_info;
	while (!in_file.is_open())
	{
		cout << "��ȡ***" + account_name + "***�˺���Ϣ�ļ�ʧ�� ��ȷ�����ļ����ڲ����ļ�����ȷ" << endl;
		Sleep(10000);
		in_file.open(fn_plus);
	}
	int item_count = 0;
	bool bIsGetTdAddress = false;
	bool bIsGetMdAddress = false;
	while (getline(in_file, s))
	{
		stringstream line(s);
		string name;
		line >> name;
		string info;
		line >> info;
		if (strcmp(name.c_str(), "����ǰ��") == 0)
		{
			account_info.TdAddress.push_back(info);
			bIsGetTdAddress = true;
		}
		else if (strcmp(name.c_str(), "����ǰ��") == 0)
		{
			account_info.MdAddress.push_back(info);
			bIsGetMdAddress = true;
		}
		else if (strcmp(name.c_str(), "�����̴���") == 0)
		{
			account_info.BrokerID = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "�˺�") == 0)
		{
			account_info.UserID = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "����") == 0)
		{
			account_info.Password = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "order�ĵ���ͷ��") == 0)
		{
			account_info.PositionFileHead = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "order�ĵ���β��") == 0)
		{
			account_info.PositionFileTail = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "�˺�����") == 0)
		{
			account_info.AccountName = info;
			item_count++;
		}
		else
		{
			;
		}
	}
	if (bIsGetTdAddress && bIsGetMdAddress)
	{
		item_count++;
		item_count++;
	}
	if (item_count < 8)
	{
		cout << "***" + account_name + "*** �˺���Ϣ�ļ���ʽ���� ���߳����˳�" << endl;
		ExitThread(0);
	}
	return account_info;
}

DWORD WINAPI TradeAccount(LPVOID lpParameter)
{
	string account_name = (*(string *)lpParameter);
	AccountInfo account_info = GetAccountInfo(account_name);
	CFTTD * pTdHandler = new CFTTD();
	CFTMD * pMdHandler = new CFTMD();
	logInfo log;
	logInfo * pLog = &log;
	string path = GetExePath();
	string log_path = path + account_info.AccountName + "\\logs";
	pLog->SetLogPath(log_path.c_str());
	pTdHandler->Init(account_info, pMdHandler, pLog);
	pMdHandler->Init(account_info, pTdHandler);
	pTdHandler->GetTargetPosition();
	//pMdHandler->test();
	clock_t start = clock();
	pTdHandler->GetCurrentPosition();
	pTdHandler->GetOrders();
	pTdHandler->PlaceOrder();
	pTdHandler->GetCurrentPosition();
	clock_t end = clock();
	WaitForSingleObject(hMutex, INFINITE);
	pTdHandler->g_pLog->printLog("������� ��ʱ%d��\n", (end - start) / CLOCKS_PER_SEC);
	printf("***%s*** ������� ��ʱ%d��\n", account_info.AccountName.c_str(), (end - start) / CLOCKS_PER_SEC);
	ReleaseMutex(hMutex);
	return 0;
}
int main(int argc, char* argv[], char *envp[])
{
	//��ȡ�˺��б�
	list<string> accounts;
	string path = GetExePath();
	string accounts_file_fn = path + "�˺��б�.txt";
	ifstream in_file(accounts_file_fn);
	if (in_file.good() && in_file.is_open())
	{
		string s;
		while (getline(in_file, s))
		{
			stringstream line(s);
			string account_s;
			line >> account_s;
			accounts.push_back(account_s);
		}
		cout << "�˺��б�����" << endl;
		for (list<string>::iterator it = accounts.begin(); it != accounts.end(); it++)
			cout << *it << endl;
		cout << endl;
	}
	else
	{
		cout << "��ȡ�˺��б�ʧ��" << endl;
		exit(-1);
	}
	//�������Ա���� ��¼CTP ������
	cout << "�����뱾��˾�����������Ա����" << endl;
	string s;
	cin >> s;
	while (strcmp(s.c_str(), "232461") != 0)
	{
		cout << "������� ����������" << endl;
		cin >> s;
	}
	cout << "������ȷ ��ʼ��½CTP" << endl;
	list<HANDLE> hThreads;
	for (list<string>::iterator it = accounts.begin(); it != accounts.end(); it++)
	{
		HANDLE hThread = CreateThread(NULL, 0, TradeAccount, &(*it), 0, NULL);
		hThreads.push_back(hThread);
	}
	//�ȴ����н����߳̽���
	for (list<HANDLE>::iterator it = hThreads.begin(); it != hThreads.end(); it++)
		WaitForSingleObject(*it, INFINITE);

	return 0;
}



