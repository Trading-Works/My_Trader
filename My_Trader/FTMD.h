#pragma once
#include "stdafx.h"
class CFTMD : public CThostFtdcMdSpi  
{
public:
	CFTMD();
	virtual ~CFTMD();
	typedef std::map<std::string, CThostFtdcDepthMarketDataField> TYP_QUTOE;
	TYP_QUTOE LastDepth;

	TThostFtdcBrokerIDType qh_BrokerID;
	TThostFtdcAddressType qh_MDAddress;
	TThostFtdcUserIDType qh_UserID;
    TThostFtdcPasswordType qh_Password;

	HANDLE hEvent;
	CThostFtdcMdApi *m_pMdApi;
    void Init(AccountInfo account_info, CFTTD* pTdHandler);
	void initSubMD();
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	double GetTradePrice(OrderStruct order);
	double GetVolume(std::string code);
	void UnsubscribeMD();
	void AccountLogout();

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	void OnFrontConnected();
	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	void OnFrontDisconnected(int nReason);
	///�������֪ͨ
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
	//�˶�����֪ͨ
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//�ǳ�֪ͨ
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//void test();
private:
    CFTTD *g_pTdHandler;
};