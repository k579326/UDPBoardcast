
// ntestDlg.h: 头文件
//

#pragma once

#include "CSDlg.h"

// CntestDlg 对话框
class CntestDlg : public CDialogEx
{
// 构造
public:
	CntestDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NTEST_DIALOG };
#endif

    void SetTabPage(int itemIndex);



	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
    CTabCtrl* m_tab;
    CSDlg* m_page[2];
    CSDlg* m_client;
    CSDlg* m_server;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
