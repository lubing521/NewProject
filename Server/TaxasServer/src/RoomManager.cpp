#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
CRoomManager::CRoomManager()
{

}

CRoomManager::~CRoomManager()
{

}

bool CRoomManager::Init()
{
	stBaseRoomConfig* pconfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,1);;
	if ( pconfig )
	{
		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init(1,(stTaxasRoomConfig*)pconfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
	}
	return true ;
}

bool CRoomManager::OnMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
		return false ;
	}

	if ( MSG_TP_ENTER_ROOM == prealMsg->usMsgType )
	{
		stMsgTaxasEnterRoom* pRel = (stMsgTaxasEnterRoom*)prealMsg ;
		CTaxasRoom* pRoom = GetRoomByID(pRel->nRoomID) ;
		if ( !pRoom )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not find room id = %d , type = %d , level = %d",pRel->nRoomID,pRel->nType,pRel->nLevel );
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 1 ;
			CTaxasServerApp::SharedGameServerApp()->SendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		if ( pRoom->IsPlayerInRoomWithSessionID(nSessionID) )
		{
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 3 ;
			CTaxasServerApp::SharedGameServerApp()->SendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}
		
		stMsgRequestTaxasPlayerData msg ;
		msg.nSessionID = nSessionID ;
		SendMsg(&msg,sizeof(msg),pRoom->GetRoomID()) ;
		return true ;
	}

	if ( MSG_TP_REQUEST_PLAYER_DATA == prealMsg->usMsgType )
	{
		if ( eSenderPort != ID_MSG_PORT_DATA )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this msg not from data server ? ") ;
			return true ;
		}

		stMsgRequestTaxasPlayerDataRet* pRet = (stMsgRequestTaxasPlayerDataRet*)prealMsg ;
		if ( pRet->nRet )
		{
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = pRet->nRet ;
			CTaxasServerApp::SharedGameServerApp()->SendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			CLogMgr::SharedLogMgr()->SystemLog("invalid session id = %d can not get player data ret = %d ", nSessionID,pRet->nRet ) ;
			return true ;
		}

		CTaxasRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why room is null server error room id = %d",pRet->nRoomID ) ;

			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 3;
			CTaxasServerApp::SharedGameServerApp()->SendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		pRoom->AddPlayer(pRet->tData);
		return true;
	}

	if ( MSG_TP_REQUEST_TAKE_IN_MONEY == prealMsg->usMsgType && eSenderPort == ID_MSG_PORT_DATA )
	{
		stMsgRequestTaxasPlayerTakeInCoinRet* pRet = (stMsgRequestTaxasPlayerTakeInCoinRet*)prealMsg ;
		CTaxasRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why not the room id = %d is null can not process msg = %d, session id = %d",pRet->nRoomID,MSG_TP_REQUEST_TAKE_IN_MONEY,nSessionID)  ;
			return true ;
		}
		pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	if ( MSG_TP_ORDER_LEAVE == prealMsg->usMsgType && eSenderPort == ID_MSG_PORT_DATA )
	{
		stMsgOrderTaxasPlayerLeave* pRet = (stMsgOrderTaxasPlayerLeave*)prealMsg ;
		CTaxasRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why not the room id = %d is null can not process msg = %d, session id = %d",pRet->nRoomID,MSG_TP_REQUEST_TAKE_IN_MONEY,nSessionID)  ;
			return true ;
		}
		pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	CTaxasRoom* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
	if ( pRoom == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find room to process id = %d ,from = %d",prealMsg->usMsgType,eSenderPort ) ;
		return  false ;
	}

	return pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
}

CTaxasRoom*CRoomManager::GetRoomByID(uint32_t nRoomID )
{
	MAP_ID_ROOM::iterator iter = m_vRooms.find(nRoomID) ;
	if ( iter != m_vRooms.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CRoomManager::SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->SendMsg(nSessionID,(char*)pmsg,nLen) ;
}