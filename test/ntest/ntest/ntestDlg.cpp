
// ntestDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "ntest.h"
#include "ntestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CntestDlg 对话框



CntestDlg::CntestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_client = new CSDlg(0);
    m_server = new CSDlg(1);
    m_tab = NULL;
    m_page[0] = m_client;
    m_page[1] = m_server;
}

void CntestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CntestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CntestDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CntestDlg 消息处理程序

BOOL CntestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// ShowWindow(SW_MAXIMIZE);
    // 
	// ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
    m_tab = (CTabCtrl*)GetDlgItem(IDC_TAB1);//dynamic_cast<CTabCtrl*>(GetDlgItem(IDC_TAB1));
    m_tab->InsertItem(0, L"客户端");
    m_tab->InsertItem(1, L"服务端");


    m_server->Create(IDD_DIALOG1);
    m_client->Create(IDD_DIALOG1);
    POINT pt = { 0, 0 };
    RECT rect;
    m_tab->GetWindowRect(&rect);
    ::ScreenToClient(m_hWnd, &pt);

    rect.left += pt.x;
    rect.right += pt.x;
    rect.top += pt.y;
    rect.bottom += pt.y;

    rect.top += 22;

    m_client->MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    m_server->MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

    SetTabPage(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CntestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CntestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CntestDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;

    SetTabPage(m_tab->GetCurSel());

    return;
}


void CntestDlg::SetTabPage(int itemIndex)
{
    for (int i = 0; i < m_tab->GetItemCount(); i++)
    {
        if (i != itemIndex)
            m_tab->HighlightItem(i, false);
        else
            m_tab->HighlightItem(i, true);
        
    }
    m_tab->SetCurSel(itemIndex);

    for (int i = 0; i < m_tab->GetItemCount(); i++)
    {
        if (i != itemIndex)
            m_page[i]->ShowWindow(SW_HIDE);
        else
            m_page[i]->ShowWindow(SW_SHOW);
    }
}



