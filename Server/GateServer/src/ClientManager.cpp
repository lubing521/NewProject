#include "ClientManager.h"
#include "GateClient.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "CommonDefine.h"
#include "ServerNetwork.h"
#include "GateServer.h"
#include "ServerMessageDefine.h"
#include "ServerNetwork.h"
#define TIME_WAIT_FOR_RECONNECTE 60
CGateClientMgr::CGateClientMgr()
{
	m_vNetWorkIDGateClientIdx.clear();
	m_vSessionGateClient.clear() ;
	m_vWaitToReconnect.clear();
	m_vGateClientReserver.clear();
	memset(m_pMsgBuffer,0,MAX_MSG_BUFFER_LEN) ;
}

CGateClientMgr::~CGateClientMgr()
{
	MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.begin();
	for ( ; iterS != m_vSessionGateClient.end(); ++iterS )
	{
		delete  iterS->second ;
	}
	m_vSessionGateClient.clear() ;

	// just clear ; object deleted in session Gate ;
	m_vNetWorkIDGateClientIdx.clear() ;

	LIST_GATE_CLIENT::iterator iter = m_vGateClientReserver.begin() ;
	for ( ; iter != m_vGateClientReserver.end(); ++iter )
	{
		delete *iter ;
	}
	m_vGateClientReserver.clear() ;

	m_vWaitToReconnect.clear();
}

bool CGateClientMgr::OnMessage( Packet* pData )
{
	// verify identify 
	stMsg* pMsg = (stMsg*)pData->_orgdata ;
	CHECK_MSG_SIZE(stMsg,pData->_len);
	if ( MSG_VERIFY_CLIENT == pMsg->usMsgType )
	{
		char* pIPInfo = CGateServer::SharedGateServer()->GetNetWorkForClients()->GetIPInfoByConnectID(pData->_connectID) ;
		stGateClient* pGateClient = GetReserverGateClient();
		if ( !pGateClient )
		{
			pGateClient = new stGateClient ;
		}

		pGateClient->Reset(CGateServer::SharedGateServer()->GenerateSessionID(),pData->_connectID,pIPInfo) ;
		AddClientGate(pGateClient);
		CLogMgr::SharedLogMgr()->SystemLog("a Client connected ip = %s Session id = %d",pGateClient->strIPAddress.c_str(),pGateClient->nSessionId ) ;
		return true;
	}

	// client reconnect ;
	if ( MSG_RECONNECT == pMsg->usMsgType )
	{
		stMsgReconnect* pRet = (stMsgReconnect*)pMsg ;
		CHECK_MSG_SIZE(stMsgReconnect,pData->_len);
		MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vWaitToReconnect.find(pRet->nSessionID);
		bool bReconnectOk = iter != m_vWaitToReconnect.end() && iter->second != NULL ;
		if ( bReconnectOk )
		{
			// remove origin 
			RemoveClientGate(iter->second);

			// bind current Client to origin sesssion id ;
			stGateClient* pNew = GetGateClientByNetWorkID(pData->_connectID);
			if ( pNew )
			{
				 MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.find(pNew->nSessionId);
				 if ( iterS == m_vSessionGateClient.end() )
				 {
					 CLogMgr::SharedLogMgr()->ErrorLog("why my session id = %d targe is null",pNew->nSessionId );
				 }
				 else
				 {
					 m_vSessionGateClient.erase(iterS);
				 }
				 pNew->nSessionId = pRet->nSessionID;
				 m_vSessionGateClient[pNew->nSessionId] = pNew ;
			}
		}

		stMsgReconnectRet msgback ;
		msgback.nRet = (bReconnectOk ? 0 : 1 ) ;
		// send msg to client ;
		CGateServer::SharedGateServer()->SendMsgToClient((char*)&msgback,sizeof(msgback),pData->_connectID,false) ;
		if ( bReconnectOk )
		{
			CLogMgr::SharedLogMgr()->SystemLog("MSGĦĦreconnected ! session id = %d",pRet->nSessionID );
		}
		return true ;
	}

	// transfer to center server 
	stGateClient* pDstClient = GetGateClientByNetWorkID(pData->_connectID) ;
	if ( pDstClient == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not send message to Center Server , client is NULL or not verified, so close the unknown connect") ;
		CGateServer::SharedGateServer()->GetNetWorkForClients()->ClosePeerConnection(pData->_connectID) ;
		return true ;
	}

	if ( CheckServerStateOk(pDstClient) == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center server is disconnected so can not send msg to it ");
		return true ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_CLIENT ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = pDstClient->nSessionId ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + pData->_len >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pData->_orgdata ;
		CLogMgr::SharedLogMgr()->ErrorLog("msg from session id = %d , is too big , cannot send , msg id = %d ",pDstClient->nSessionId,pmsg->usMsgType) ;
		return true ;
	}
	memcpy(m_pMsgBuffer,&msgTransData,nLne);
	memcpy(m_pMsgBuffer + nLne , pData->_orgdata,pData->_len );
	nLne += pData->_len ;
	CGateServer::SharedGateServer()->SendMsgToCenterServer(m_pMsgBuffer,nLne);
	return true ;
}

void CGateClientMgr::OnServerMsg( const char* pRealMsgData, uint16_t nDataLen,uint32_t uTargetSessionID )
{
	stGateClient* pClient = GetGateClientBySessionID(uTargetSessionID) ;
	if ( NULL == pClient )
	{
		stMsg* pReal = (stMsg*)pRealMsgData ;
		CLogMgr::SharedLogMgr()->ErrorLog("big error !!!! can not send msg to session id = %d , client is null , msg = %d",uTargetSessionID,pReal->usMsgType  ) ;
		return  ;
	}

	if ( pClient->tTimeForRemove )
	{
		CLogMgr::SharedLogMgr()->SystemLog("client is waiting for reconnected") ;
		return ;
	}

	CGateServer::SharedGateServer()->SendMsgToClient(pRealMsgData,nDataLen,pClient->nNetWorkID ) ;
}

void CGateClientMgr::OnNewPeerConnected(CONNECT_ID nNewPeer, ConnectInfo* IpInfo)
{
	if ( IpInfo )
	{
		CLogMgr::SharedLogMgr()->PrintLog("a peer connected ip = %s ,port = %d",IpInfo->strAddress,IpInfo->nPort ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->PrintLog("a peer connected ip = NULL" ) ;
	}
	
	//stMsg msg ;
	//msg.cSysIdentifer = ID_MSG_VERIFY ;
	//msg.usMsgType = MSG_VERIFY_GATE ;
	//// send msg to peer ;
	//CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),pData->guid,false) ;
}

void CGateClientMgr::OnPeerDisconnected(CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo  )
{
	// client disconnected ;
	stGateClient* pDstClient = GetGateClientByNetWorkID(nPeerDisconnected) ;
	if ( pDstClient )
	{
		if ( pDstClient->tTimeForRemove )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("already wait to reconnected");
			return ;
		}

		pDstClient->tTimeForRemove = time(NULL) + TIME_WAIT_FOR_RECONNECTE ;
		m_vWaitToReconnect[pDstClient->nSessionId] = pDstClient;

		if ( IpInfo )
		{
			CLogMgr::SharedLogMgr()->SystemLog("client disconnected ip = %s, port = %d, wait for reconnect",IpInfo->strAddress,IpInfo->nPort ) ;
		}
		return ;
	}

	if ( IpInfo )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("not verify peer disconnected ip = %s, port = %d",IpInfo->strAddress,IpInfo->nPort ) ;
	}
}

void CGateClientMgr::AddClientGate(stGateClient* pGateClient )
{
	if ( m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID) != m_vNetWorkIDGateClientIdx.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this pos already have data client") ;
		m_vNetWorkIDGateClientIdx.erase(m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID));
	}

	if ( m_vSessionGateClient.find(pGateClient->nSessionId) != m_vSessionGateClient.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this pos session id = %d had client data",pGateClient->nSessionId) ;
		m_vSessionGateClient.erase(m_vSessionGateClient.find(pGateClient->nSessionId));
	}

	m_vNetWorkIDGateClientIdx[pGateClient->nNetWorkID] = pGateClient ;
	m_vSessionGateClient[pGateClient->nSessionId] = pGateClient ;
}

void CGateClientMgr::RemoveClientGate(stGateClient* pGateClient )
{
	if ( pGateClient == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why remove a null client ") ;
		return ;
	}

	MAP_NETWORKID_GATE_CLIENT::iterator iterN = m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID) ;
	if ( iterN != m_vNetWorkIDGateClientIdx.end() )
	{
		m_vNetWorkIDGateClientIdx.erase(iterN) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find net work id = %d to remove",pGateClient->nNetWorkID ) ;
	}
	
	MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.find(pGateClient->nSessionId );
	if ( iterS != m_vSessionGateClient.end() )
	{
		m_vSessionGateClient.erase(iterS) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find session id = %d to remove",pGateClient->nSessionId ) ;
	}

	iterS = m_vWaitToReconnect.find(pGateClient->nSessionId) ;
	if ( iterS != m_vWaitToReconnect.end() )
	{
		m_vWaitToReconnect.erase(iterS) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why can not find session id = %d to remove from vWaiReconecte",pGateClient->nSessionId) ;
	}
	
	pGateClient->Reset(0,INVALID_CONNECT_ID,NULL) ;
	m_vGateClientReserver.push_back(pGateClient) ;
}

stGateClient* CGateClientMgr::GetReserverGateClient()
{
	stGateClient* pGateClient = NULL ;
	if ( m_vGateClientReserver.empty() == false )
	{
		pGateClient = m_vGateClientReserver.front() ;
		m_vGateClientReserver.erase(m_vGateClientReserver.begin()) ;
	}
	return pGateClient ;
}

stGateClient* CGateClientMgr::GetGateClientBySessionID(unsigned int nSessionID)
{
	MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vSessionGateClient.find(nSessionID) ;
	if ( iter == m_vSessionGateClient.end() )
		return NULL ;
	return iter->second ;
}

void CGateClientMgr::UpdateReconectClientLife()
{
	if ( m_vWaitToReconnect.empty() )
	{
		return ;
	}

	time_t tNow = time(NULL) ;
	LIST_GATE_CLIENT vWillRemove ;
	MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vWaitToReconnect.begin();
	for ( ; iter != m_vWaitToReconnect.end(); ++iter )
	{
		if ( iter->second == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this null client wait reconnect");
			continue;
		}

		if ( iter->second->tTimeForRemove == 0 )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("big error , timeForRemove can not be 0 ") ;
		}

		if ( iter->second->tTimeForRemove <= tNow )
		{
			vWillRemove.push_back(iter->second) ;
		}
	}

	// do remove 
	LIST_GATE_CLIENT::iterator iterRemove = vWillRemove.begin() ;
	for ( ; iterRemove != vWillRemove.end(); ++iterRemove )
	{
		stGateClient* p = *iterRemove ;
		if ( p == NULL )
		{
			continue;
		}
		// tell other server the peer disconnect
		{
			stMsgClientDisconnect msgdis ;
			msgdis.nSeesionID = p->nSessionId ;
			CGateServer::SharedGateServer()->SendMsgToCenterServer((char*)&msgdis,sizeof(msgdis));
		}

		// do remove 
		CLogMgr::SharedLogMgr()->SystemLog("session id = %d , ip = %s , wait reconnect time out ,do exit game",p->nSessionId,p->strIPAddress.c_str()) ;
		RemoveClientGate(p);
	}
	vWillRemove.clear();
}

stGateClient* CGateClientMgr::GetGateClientByNetWorkID(CONNECT_ID& nNetWorkID )
{
	MAP_NETWORKID_GATE_CLIENT::iterator iter = m_vNetWorkIDGateClientIdx.find(nNetWorkID) ;
	if ( iter != m_vNetWorkIDGateClientIdx.end() )
		return iter->second ;
	return NULL ;
}

bool CGateClientMgr::CheckServerStateOk( stGateClient* pClient)
{
	bool b = CGateServer::SharedGateServer()->IsCenterServerConnected() ;
	if ( b )
	{
		return true ;
	}
	stMsgServerDisconnect msg ;
	msg.nServerType = eSvrType_Center ;
	CGateServer::SharedGateServer()->SendMsgToClient((char*)&msg,sizeof(msg),pClient->nNetWorkID ) ;
	return false ;
}
