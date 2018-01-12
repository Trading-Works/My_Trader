#include "stdafx.h"
using namespace std;
CFTMD::CFTMD()
{

}

CFTMD::~CFTMD()
{
	
}

void CFTMD::Init(AccountInfo account_info, CFTTD* pTdHandler)
{
	// ����һ��CThostFtdcMdApiʵ��
	memset(qh_BrokerID, 0, sizeof(qh_BrokerID));
	memset(qh_MDAddress, 0, sizeof(qh_MDAddress));
	memset(qh_UserID, 0, sizeof(qh_UserID));
	memset(qh_Password, 0, sizeof(qh_Password));
	strcpy_s(qh_BrokerID, account_info.BrokerID.c_str());
	strcpy_s(qh_UserID, account_info.UserID.c_str());
	strcpy_s(qh_Password, account_info.Password.c_str());
	g_pTdHandler = pTdHandler;

	m_pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
	m_pMdApi->RegisterSpi(this);
	for (list<string>::iterator it = account_info.MdAddress.begin(); it != account_info.MdAddress.end(); it++)
	{
		strcpy_s(qh_MDAddress, (*it).c_str());
		m_pMdApi->RegisterFront(qh_MDAddress);
	}
	hEvent = CreateEvent(NULL, false, false, NULL);
	m_pMdApi->Init();
	if (WaitForSingleObject(hEvent, 30000) == WAIT_TIMEOUT)
		printf("***%s*** ����ǰ�˵�¼���� ���߳����˳�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	else
		printf("***%s*** ����ǰ�˵�½�ɹ�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	//�ȴ����鶩�����
	int waitingtimes = 0;
	while (LastDepth.size() < g_pTdHandler->Instruments.size())
	{
	
		Sleep(500);
		waitingtimes++;
		if (waitingtimes > 4)
			break;
		cout << account_info.AccountName<<g_pTdHandler->Instruments.size()- LastDepth.size() << "  "<<waitingtimes<<endl;
	}

}

void CFTMD::OnFrontConnected()
{
	CThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID, qh_BrokerID);
	strcpy_s(reqUserLogin.UserID, qh_UserID);
	strcpy_s(reqUserLogin.Password, qh_Password);
	int login = m_pMdApi->ReqUserLogin(&reqUserLogin,1);
}
void CFTMD::OnFrontDisconnected(int nReason)
{
	printf("***%s*** �������ӶϿ� ԭ�����:%d\n", g_pTdHandler->g_AccountInfo.AccountName.c_str(), nReason);
}
void CFTMD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo->ErrorID == 0)
		initSubMD();
	else
		printf("***%s*** ����ǰ�˵�½���� ErrorID=%d ErrorMsg=%s ��ǰ����=%s\n",g_pTdHandler->g_AccountInfo.AccountName.c_str(), pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->TradingDay);
}

void CFTMD::initSubMD()
{
    //���ݺ�Լ�б�������
	int n_count = g_pTdHandler->Instruments.size();
	char ** codes = new char*[n_count];
	TThostFtdcInstrumentIDType* InstrumentIDs = new TThostFtdcInstrumentIDType[n_count];
	int i = 0;
	for (list<CThostFtdcInstrumentField>::iterator it = g_pTdHandler->Instruments.begin(); it!=g_pTdHandler->Instruments.end(); it++)
	{
		strcpy_s(InstrumentIDs[i], it->InstrumentID);
		codes[i] = InstrumentIDs[i];
		i++;
		
	}
	m_pMdApi->SubscribeMarketData(codes, n_count);

	delete[] codes;
	delete[] InstrumentIDs;
}
//void CFTMD::test()
//{
//	cout <<"LastDepth['jjssd'].LastPrice " <<LastDepth["jjssd"].LastPrice << endl;
//}
void CFTMD::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast)
		SetEvent(hEvent);
}
void CFTMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{
	CThostFtdcDepthMarketDataField *pMD;
	LastDepth[pDepthMarketData->InstrumentID] = *pDepthMarketData;
	//g_pTdHandler->g_pLog->printLog("ID��%s \n", pDepthMarketData->InstrumentID);
	pMD = &LastDepth[pDepthMarketData->InstrumentID];
	pMD->LastPrice = (pMD->LastPrice > 10000000.0) ? 0 : pMD->LastPrice;                          ///���¼�
	pMD->OpenPrice = (pMD->OpenPrice > 10000000.0) ? pMD->LastPrice : pMD->OpenPrice;             ///����
	pMD->HighestPrice = (pMD->HighestPrice > 10000000.0) ? pMD->LastPrice : pMD->HighestPrice;    ///��߼�
	pMD->LowestPrice = (pMD->LowestPrice > 10000000.0) ? pMD->LastPrice : pMD->LowestPrice;       ///��ͼ�
	pMD->BidPrice1 = (pMD->BidPrice1 > 10000000.0) ? pMD->LastPrice : pMD->BidPrice1;             ///�����һ
	pMD->AskPrice1 = (pMD->AskPrice1 > 10000000.0) ? pMD->LastPrice : pMD->AskPrice1;             ///������һ
	pMD->AveragePrice = (pMD->AveragePrice > 10000000.0) ? pMD->LastPrice : pMD->AveragePrice;    ///���վ�
}
double CFTMD::GetTradePrice(OrderStruct order)
{
	double price = 0.001;
	if (LastDepth.size() == 0)
	{
		printf("***%s*** CFTTD::GetTradePrice ��ȡ�������ݴ��� ���߳����˳�\n",g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else if (order.direction == THOST_FTDC_DEN_Buy)
		price = LastDepth[order.code].AskPrice1;
	else if (order.direction == THOST_FTDC_DEN_Sell)
		price = LastDepth[order.code].BidPrice1;
	else
		;
	//����δ���嵽�ĺ�Լ���趨�۸�Ϊ0.001��С��Ŀǰ���к�Լ����С�䶯��λ������������Ч��
	if (price > 10000 * 1000)
		price = 0.001;
	else if (price < -10000 * 1000)
		price = 0.001;
	return price;
}

double CFTMD::GetVolume(string code)
{
	if (LastDepth.size() == 0)
	{
		printf("***%s*** CFTTD::GetVolume ��ȡ�������ݴ��� ���߳����˳�\n",g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	return LastDepth[code].Volume;
}

void CFTMD::AccountLogout()
{
	UnsubscribeMD();
	CThostFtdcUserLogoutField UserLogoutField;
	memset(&UserLogoutField, 0, sizeof(UserLogoutField));
	CThostFtdcUserLogoutField * pUserLogoutField = &UserLogoutField;
	strcpy_s(pUserLogoutField->BrokerID, g_pTdHandler->g_AccountInfo.BrokerID.c_str());
	strcpy_s(pUserLogoutField->UserID, g_pTdHandler->g_AccountInfo.UserID.c_str());
	ResetEvent(hEvent);
	m_pMdApi->ReqUserLogout(pUserLogoutField, 1);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_TIMEOUT)
	{
		g_pTdHandler->g_pLog->printLog("����ǰ�˵ǳ�ʧ��\n");
		printf("***%s*** ����ǰ�˵ǳ�ʧ�� ���߳����˳�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else
	{
		g_pTdHandler->g_pLog->printLog("����ǰ�˵ǳ�\n");
		printf("***%s*** ����ǰ�˵ǳ�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	}

}

void CFTMD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
		SetEvent(hEvent);
}

void CFTMD::UnsubscribeMD()
{
	//ȡ�������ĵ������б�
	int n_count = g_pTdHandler->Instruments.size();
	char ** codes = new char*[n_count];
	TThostFtdcInstrumentIDType* InstrumentIDs = new TThostFtdcInstrumentIDType[n_count];
	int i = 0;
	for (list<CThostFtdcInstrumentField>::iterator it = g_pTdHandler->Instruments.begin(); it != g_pTdHandler->Instruments.end(); it++)
	{
		strcpy_s(InstrumentIDs[i], it->InstrumentID);
		codes[i] = InstrumentIDs[i];
		i++;
	}
	ResetEvent(hEvent);
	m_pMdApi->UnSubscribeMarketData(codes, n_count);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_TIMEOUT)
	{
		g_pTdHandler->g_pLog->printLog("�˶�����ʧ��\n");
		printf("***%s*** �˶�����ʧ�� ���߳����˳�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else
	{
		g_pTdHandler->g_pLog->printLog("�˶�����ɹ�\n");
		printf("***%s*** �˶�����ɹ�\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	}
	delete[] codes;
	delete[] InstrumentIDs;
}

void CFTMD::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast)
		SetEvent(hEvent);
}