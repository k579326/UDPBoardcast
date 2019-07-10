



#pragma once

#include <afx.h>

#include <vector>
#include <map>



using namespace std;

class SendHelper
{
public:
    SendHelper() : status(false) {
    }
    ~SendHelper() { }

    bool GetSenderStatus(){ 
        return status;
    }
    void SetSenderStatus(bool s){
        status = s;
    }

    void SetText(CString text){
        sendText = text;
    }
    CString GetText(){
        return sendText;
    }

private:

    bool status;
    CString sendText;
};



