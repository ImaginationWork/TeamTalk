/*******************************************************************************
 *  @file      SessionLayout.h 2014\12\31 14:05:29 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/
#ifndef SESSIONLAYOUT_6BC9730E_47F6_4BCB_936D_AC034AA10DFF_H__
#define SESSIONLAYOUT_6BC9730E_47F6_4BCB_936D_AC034AA10DFF_H__

#include "DuiLib/UIlib.h"
#include "GlobalDefine.h"
#include "Modules/ModuleObserver.h"

class UIIMEdit;
#include <memory>
#include <vector>
/******************************************************************************/
const UInt32 INIT_FOCUS_TIMER_ID = 10088;
const UInt8	FETCH_MSG_COUNT_PERTIME = 10;

using namespace DuiLib;
class MessageEntity;
class EmotionDialog;

class MixedMsg;

namespace module
{
	struct IHttpOperation;
}
/**
* The class <code>SessionLayout</code>
*
*/
class SessionLayout :public CHorizontalLayoutUI, public INotifyUI, public CWebBrowserEventHandler
{
public:
    /** @name Constructors and Destructor*/
    //@{
    /**
     * Constructor 
     */

	SessionLayout(const std::string& sId, CPaintManagerUI& paint_manager);
    /**
     * Destructor
     */
    virtual ~SessionLayout();
    //@}

public:
	virtual void DoInit();
	virtual void DoEvent(TEventUI& event);
	virtual void Notify(TNotifyUI& msg);
	void OnWindowInitialized();

    void DocmentComplete(IDispatch *pDisp, VARIANT *&url); //打开会话已经，等浏览器初始化完成，加载未读消息

	virtual HRESULT STDMETHODCALLTYPE TranslateUrl(
		/* [in] */ DWORD dwTranslate,
		/* [in] */ OLECHAR __RPC_FAR *pchURLIn,
		/* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
	virtual void NewWindow2(VARIANT_BOOL *&Cancel, BSTR bstrUrl);

	//语音播放动画操作
	BOOL StopPlayingAnimate(std::string& sAudioPlayingID);
	BOOL StartPlayingAnimate(std::string sAudioPlayingID);
public:
	/**@name MKO*/
	//@{
	void MKOForEmotionModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForGroupModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	//@}

public:
	void SendMsg();
	void UpdateRunTimeMsg();	//更新未读消息
	void UpdateSendMsgShortcut();//更新快捷键
	void UpdateBottomLayout();
	void FreshGroupMemberAvatar(IN const std::string& sID);		//刷新群成员的在线状态
	void FreshAllGroupMemberAvatar();
	void DoDisplayHistoryMsgToIE(std::vector<MessageEntity>& msgList, BOOL scrollBottom);

	void OnSendImageCallback(std::shared_ptr<void> param);//发送图片返回
    void OnFinishScreenCapture(__in LPCTSTR lpFilePath);
    UInt32 GetGroupItemCnt(void);
private:
	/**
	 * 展示群成员
	 *
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	void _UpdateGroupMembersList();
	void _AddGroupMemberToList(IN const std::string& sID,IN const BOOL bCreator);
	void _UpdateSearchRsultList(IN const std::vector<std::string>& nameList);
    void _LoadFirstOpenedMsg(void);//加载第一次打开对话康的消息（优先未读消息，其次历史消息）
	BOOL _DisplayUnreadMsg();
	void _DisplayHistoryMsgToIE(UInt32 nMsgCount, BOOL scrollBottom);
	BOOL _DisplayMsgToIE(IN MessageEntity msg);
	void _DisplaySysMsg(IN CString strID);
	void _SendSessionMsg(IN MixedMsg mixMsg);//投递消息
	void _SendImage(CString& csFilePath);
	void _CreateMenu(IN const TNotifyUI& msg);
	void _GetGroupNameListByShortName(IN const CString& sShortName, OUT std::vector<string>& nameList);

	/**
	 * 发送已读确认
	 *
	 * @param   IN MessageEntity msg
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	void _AsynSendReadAck(IN MessageEntity& msg);	
	void _DafoNetWorkPicMsg(OUT MixedMsg& mixMsg);//大佛彩蛋，消息动态外链
public:
	CPaintManagerUI&	m_paint_manager;

    CWebBrowserUI*		m_pWebBrowser = nullptr;//聊天显示框
    BOOL                m_bDocumentReady = FALSE;
    UIIMEdit*			m_pInputRichEdit = nullptr;

    CContainerUI*		m_pRightContainer = nullptr;
    CListUI*			m_pGroupMemberList = nullptr;
    CEditUI*			m_pSearchEdit = nullptr;
    CListUI*			m_pSearchResultList = nullptr;

    CTextUI*			m_pSendDescription = nullptr;	// ctrl+enter /enter

    CButtonUI*			m_pBtnSendMsg = nullptr;
    CButtonUI*			m_pBtnClose = nullptr;

    CButtonUI*			m_pBtnEmotion = nullptr;		//表情
    CButtonUI*			m_pBtnSendImage = nullptr;	//发送图片
    CButtonUI*			m_pBtnScreenShot = nullptr;	//截屏
    CButtonUI*			m_pBtnshock = nullptr;
    CButtonUI*			m_pBtnsendfile = nullptr;
    CButtonUI*			m_pBtnbanGroupMsg = nullptr;
    CButtonUI*			m_pBtndisplayGroupMsg = nullptr;
    CButtonUI*			m_pBtnadduser = nullptr;//添加讨论组成员/创建讨论组

    CHorizontalLayoutUI*	m_bottomLayout = nullptr;

    //std::string				m_sId;								//会话Id
    std::string				m_sId="1";								//会话Id
	std::vector<MixedMsg>	m_SendingMixedMSGList;				//正在发送的图文混排消息
	time_t					m_tShakeWindow;
	CString					m_csTobeTranslateUrl;				//IE控件转向URL

	BOOL					m_bGroupSession;					//作为输入状态判断用，群是不需要发状态的
	BOOL					m_bWritingTimerExist;				//正在输入的timer是否存在
	module::IHttpOperation*	m_pSendImgHttpOper = 0;


private:
    bool isInitialzied = false;

};
/******************************************************************************/



class SessionContainer : public CTabLayoutUI, public IDialogBuilderCallback{
public:
    SessionContainer(CPaintManagerUI& paint_manager);
    void AddSession(const std::string &sId);
    bool haveSessionLayoutBySId(const std::string &sId);
    bool UpdateRunTimeMsg(const std::string &sId);
    bool DoDisplayHistoryMsgToIE(const std::string &sid, std::vector<MessageEntity>& msgList, IN const BOOL scrollBottom);
public:
    virtual CControlUI* CreateControl(LPCTSTR pstrClass) override;
private:
    CPaintManagerUI &paint_manager;
    SessionLayout *curr = nullptr;
    std::string tempSID;
    std::vector<CControlUI *> layoutList;
};

#endif// SESSIONLAYOUT_6BC9730E_47F6_4BCB_936D_AC034AA10DFF_H__
