// CSDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "ntest.h"
#include "CSDlg.h"
#include "afxdialogex.h"


// CSDlg 对话框

IMPLEMENT_DYNAMIC(CSDlg, CDialogEx)

CSDlg::CSDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
    //CListCtl* list = GetDlgItem(IDC_LIST1);
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


void CSDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);

    // TODO: 在此处添加消息处理程序代码
}
