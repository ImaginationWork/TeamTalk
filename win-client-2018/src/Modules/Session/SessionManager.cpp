/******************************************************************************* 
 *  @file      SessionManager.cpp 2015\1\8 12:57:31 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "SessionManager.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IDatabaseModule.h"
#include <algorithm>
/******************************************************************************/

// -----------------------------------------------------------------------------
//  SessionManager: Public, Constructor

SessionEntityManager::SessionEntityManager()
{
	m_globalUpdatedTime = module::getSysConfigModule()->getRecentSessionLatestUpdateTime();
}

// -----------------------------------------------------------------------------
//  SessionManager: Public, Destructor

SessionEntityManager::~SessionEntityManager()
{
	_removeAllSessionEntity();
	module::getSysConfigModule()->saveRecentSessionLatestUpdateTime(m_globalUpdatedTime);
}

SessionEntityManager* SessionEntityManager::getInstance()
{
	static SessionEntityManager manager;
	return &manager;
}
void SessionEntityManager::setSessionEntity(IN const module::SessionEntity& sessionInfo)
{
	module::SessionEntity* pSessionEntity = createSessionEntity(sessionInfo.sessionID);
	if (nullptr != pSessionEntity)
	{
		CAutoLock lock(&m_lock);
		*pSessionEntity = sessionInfo;
		auto iterSessionID = std::find_if(m_vecRecentSession.begin(), m_vecRecentSession.end(),
			[=](std::string sessionId)
		{
			return sessionId == sessionInfo.sessionID;
		});
		if (iterSessionID == m_vecRecentSession.end())
		{
			m_vecRecentSession.push_back(sessionInfo.sessionID);
		}	
	}
}
module::SessionEntity* SessionEntityManager::createSessionEntity(const std::string& sId)
{
	module::SessionEntity* pSessionEntity = getSessionEntityBySId(sId);
	if (!pSessionEntity)
	{
		pSessionEntity = new module::SessionEntity;
		pSessionEntity->sessionType = _getSessionType(sId);
		pSessionEntity->sessionID = sId;
		pSessionEntity->setUpdatedTimeByServerTime();
		{
			CAutoLock lock(&m_lock);
			m_mapSessionEntity[sId] = pSessionEntity;
		}
	}
	return pSessionEntity;
}

module::SessionEntity* SessionEntityManager::getSessionEntityBySId(IN const std::string& sId)
{
	CAutoLock lock(&m_lock);
	auto iter = m_mapSessionEntity.find(sId);
	if (iter != m_mapSessionEntity.end())
	{
		return iter->second;
	}
	return nullptr;
}

BOOL SessionEntityManager::removeSessionEntity(const std::string& sId)
{
	CAutoLock lock(&m_lock);
	auto iterMap = m_mapSessionEntity.find(sId);
	if (iterMap == m_mapSessionEntity.end())
		return FALSE;
	module::SessionEntity* pSessionEntity = iterMap->second;
	delete  pSessionEntity;
	pSessionEntity = 0;
	m_mapSessionEntity.erase(iterMap);

	return TRUE;
}

void SessionEntityManager::_removeAllSessionEntity()
{
	CAutoLock lock(&m_lock);
	auto iter = m_mapSessionEntity.begin();
	for (; iter != m_mapSessionEntity.end(); ++iter)
	{
		delete iter->second;
		iter->second = 0;
	}
	m_mapSessionEntity.clear();
}

UInt8 SessionEntityManager::_getSessionType(IN const std::string& sID)
{
	if (sID.empty())
	{
		return module::SESSION_ERRORTYPE;
	}
	else if (std::string::npos == sID.find("group_"))
	{
		return module::SESSION_USERTYPE;
	}
	else
	{
		return module::SESSION_GROUPTYPE;
	}
}
void SessionEntityManager::getRecentSessionList(OUT std::vector<std::string>& vecRecentSession)
{
	vecRecentSession = m_vecRecentSession;
}

BOOL SessionEntityManager::loadSessionEntityFromDB()
{
	std::vector<module::SessionEntity> sessionList;
	if (!module::getDatabaseModule()->sqlGetAllRecentSessionInfo(sessionList))
	{
		LOG__(ERR, _T("db get all recenet session failed"));
		return FALSE;
	}
	for (module::SessionEntity sessionInfo:sessionList)
	{
		setSessionEntity(sessionInfo);
	}

	return TRUE;
}
