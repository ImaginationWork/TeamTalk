#include "../DBPool.h"
#include "../HttpClient.h"
#include "ImageModel.h"

using namespace std;

/**
 *  这只语音存储的url地址
 *
 *  @param strFileSite 上传的url
 */
void CImageModel::setUrl(string& strFileSite)
{
    m_strFileSite = strFileSite;
    if(m_strFileSite[m_strFileSite.length()] != '/')
    {
        m_strFileSite += "/";
    }
}

/**
 *  读取语音消息
 *
 *  @param nImageId 语音的Id
 *  @param cMsg     语音消息，引用
 *
 *  @return bool 成功返回true，失败返回false
 */
bool CImageModel::readImages(list<IM::BaseDefine::MsgInfo>& lsMsg)
{
    if(lsMsg.empty())
    {
        return true;
    }
    bool bRet = false;
    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        for (auto it=lsMsg.begin(); it!=lsMsg.end(); )
        {
            IM::BaseDefine::MsgType nType = it->msg_type();
            if((IM::BaseDefine::MSG_TYPE_GROUP_AUDIO ==  nType) || (IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO == nType) || (IM::BaseDefine::MSG_TYPE_SINGLE_IMAGE == nType))
            {
                string strSql = "select * from IMImage where id=" + it->msg_data();
                CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
                if (pResultSet)
                {
                    while (pResultSet->Next()) {
                        uint32_t nCostTime = pResultSet->GetInt("duration");
                        uint32_t nSize = pResultSet->GetInt("size");
                        string strPath = pResultSet->GetString("path");
                        readImageContent(nCostTime, nSize, strPath, *it);
                    }
                    ++it;
                    delete pResultSet;
                }
                else
                {
                    log("no result for sql:%s", strSql.c_str());
                    it = lsMsg.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
        pDBManger->RelDBConn(pDBConn);
        bRet = true;
    }
    else
    {
        log("no connection for teamtalk_slave");
    }
    return bRet;
}

/**
 *  存储语音消息
 *
 *  @param nFromId     发送者Id
 *  @param nToId       接收者Id
 *  @param nCreateTime 发送时间
 *  @param pImageData  指向语音消息的指针
 *  @param nImageLen   语音消息的长度
 *
 *  @return 成功返回语音id，失败返回-1
 */
int CImageModel::saveImageInfo(uint32_t nFromId, uint32_t nToId, uint32_t nCreateTime, const char* pImageData, uint32_t nImageLen)
{
	// parse audio data
	uint32_t nCostTime = CByteStream::ReadUint32((uchar_t*)pImageData);
	uchar_t* pRealData = (uchar_t*)pImageData + 4;
	uint32_t nRealLen = nImageLen - 4;
    int nImageId = -1;
    
	CHttpClient httpClient;
	string strPath = httpClient.UploadByteFile(m_strFileSite, pRealData, nRealLen);
	if (!strPath.empty())
    {
        CDBManager* pDBManager = CDBManager::getInstance();
        CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
        if (pDBConn)
        {
            uint32_t nStartPos = 0;
            string strSql = "insert into IMImage(`fromId`, `toId`, `path`, `size`, `duration`, `created`) "\
            "values(?, ?, ?, ?, ?, ?)";
            replace_mark(strSql, nFromId, nStartPos);
            replace_mark(strSql, nToId, nStartPos);
            replace_mark(strSql, strPath, nStartPos);
            replace_mark(strSql, nRealLen, nStartPos);
            replace_mark(strSql, nCostTime, nStartPos);
            replace_mark(strSql, nCreateTime, nStartPos);
            if (pDBConn->ExecuteUpdate(strSql.c_str()))
            {
                nImageId = pDBConn->GetInsertId();
                log("audioId=%d", nImageId);
            } else
            {
                log("sql failed: %s", strSql.c_str());
            }
            pDBManager->RelDBConn(pDBConn);
        }
        else
        {
            log("no db connection for teamtalk_master");
        }
	}
    else
    {
        log("upload file failed");
    }
	return nImageId;
}

/**
 *  读取语音的具体内容
 *
 *  @param nCostTime 语音时长
 *  @param nSize     语音大小
 *  @param strPath   语音存储的url
 *  @param cMsg      msg结构体
 *
 *  @return 成功返回true，失败返回false
 */
bool CImageModel::readImageContent(uint32_t nCostTime, uint32_t nSize, const string& strPath, IM::BaseDefine::MsgInfo& cMsg)
{
	if (strPath.empty() || nCostTime == 0 || nSize == 0) {
		return false;
	}

	// 分配内存，写入音频时长
    ImageMsgInfo cImageMsg;
    uchar_t* pData = new uchar_t [4 + nSize];
	cImageMsg.data = pData;
	CByteStream::WriteUint32(cImageMsg.data, nCostTime);
    cImageMsg.data_len = 4;
    cImageMsg.fileSize = nSize;

	// 获取音频数据，写入上面分配的内存
    CHttpClient httpClient;
    if(!httpClient.DownloadByteFile(strPath, &cImageMsg))
    {
        delete [] pData;
        return false;
    }

    log("download_path=%s, data_len=%d", strPath.c_str(), cImageMsg.data_len);
    cMsg.set_msg_data((const char*)cImageMsg.data, cImageMsg.data_len);
    
    delete [] pData;
    return true;
}
