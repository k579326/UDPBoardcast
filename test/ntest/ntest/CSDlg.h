#pragma once

#include "model.h"
// CSDlg 对话框

class CSDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSDlg)

public:
	CSDlg(int type, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

    virtual BOOL OnInitDialog();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);


public:

    void log();

    const int dlgType; // client 0, server 1

private:

    CButton* startBt;
    CButton* stopBt;

    CEdit* sendBox;
    CButton* singleSendBt;
    CButton* repeatSendBt;
    CButton* stopRepeatBt;
    CComboBox* connIdComb;
    SendHelper sHelper;


    CEdit* pushBox;
    CButton* pushBt;

    CEdit* respBox;
    CButton* respBt;

    CStatic* connCountTb;

    CListCtrl* connList;
    CListCtrl* logInfoList;

    CString respText;
    CString pushText;

};







