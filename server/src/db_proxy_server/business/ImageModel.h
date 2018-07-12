#pragma once

#include <list>
#include <map>
#include "public_define.h"
#include "util.h"
#include "IM.BaseDefine.pb.h"

using namespace std;

template <typename T>
class Singleton {
public:
    static T* getInstance() {
        static T *_instance = nullptr;
        if (_instance == nullptr) {
            _instance = new T;
        }
        return _instance;
    }

protected:
    virtual ~Singleton() { }
};


class CImageModel : public Singleton<CImageModel> {
public:
    CImageModel() {}
	virtual ~CImageModel(){}

    void setUrl(string& strFileUrl);
    
    bool readImages(list<IM::BaseDefine::MsgInfo>& lsMsg);
    
    int saveImageInfo(uint32_t nFromId, uint32_t nToId, uint32_t nCreateTime, const char* pImageData, uint32_t nImageLen);

private:
//    void GetImagesInfo(uint32_t nImageId, IM::BaseDefine::MsgInfo& msg);
    bool readImageContent(uint32_t nCostTime, uint32_t nSize, const string& strPath, IM::BaseDefine::MsgInfo& msg);
    
private:
	static CImageModel*	m_pInstance;
    string m_strFileSite;
};
