// CSDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "ntest.h"
#include "CSDlg.h"
#include "afxdialogex.h"


// CSDlg 对话框

IMPLEMENT_DYNAMIC(CSDlg, CDialogEx)

CSDlg::CSDlg(int type, CWnd* pParent) : CDialogEx(IDD_DIALOG1, pParent), dlgType(type)
{
    startBt = NULL;
    stopBt = NULL;

    sendBox = NULL;
    singleSendBt = NULL;
    repeatSendBt = NULL;
    connIdComb = NULL;

    pushBox = NULL;
    pushBt = NULL;

    respBox = NULL;
    respBt = NULL;

    connCountTb = NULL;
    connList = NULL;
    logInfoList = NULL;

}

CSDlg::~CSDlg()
{
}

void CSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSDlg, CDialogEx)
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CSDlg 消息处理程序
BOOL CSDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    startBt = (CButton*)GetDlgItem(IDC_BUTTON1);
    stopBt = (CButton*)GetDlgItem(IDC_BUTTON2);

    sendBox = (CEdit*)GetDlgItem(IDC_EDIT1);
    singleSendBt = (CButton*)GetDlgItem(IDC_BT_SEND2);
    repeatSendBt = (CButton*)GetDlgItem(IDC_BT_SEND);
    stopRepeatBt = (CButton*)GetDlgItem(IDC_BT_SEND3);
    connIdComb = (CComboBox*)GetDlgItem(IDC_COMBO1);
    pushBox = (CEdit*)GetDlgItem(IDC_EDIT2);
    pushBt = (CButton*)GetDlgItem(IDC_BT_PUSH);

    respBox = (CEdit*)GetDlgItem(IDC_EDIT3);
    respBt = (CButton*)GetDlgItem(IDC_BT_SET_RESP_CONTENT);

    connCountTb = (CStatic*)GetDlgItem(IDC_STATIC);
    connList = (CListCtrl*)GetDlgItem(IDC_LIST1);
    logInfoList = (CListCtrl*)GetDlgItem(IDC_LIST3);


    startBt->EnableWindow(FALSE);


    return false;
}

void CSDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);

    // TODO: 在此处添加消息处理程序代码
}
