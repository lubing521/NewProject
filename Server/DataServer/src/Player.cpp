#include "Player.h"
#include "LogManager.h"
#include "PlayerManager.h"
#include "GameServerApp.h"
#include "PlayerBaseData.h"
#include "PlayerMail.h"
#include "PlayerItem.h"
#include "PlayerMission.h"
#include "Timer.h"
#include "PlayerShop.h"
#include "ServerMessageDefine.h"
#include "RobotManager.h"
#define TIME_SAVE 60*20
CPlayer::CPlayer( )
{
	m_nUserUID = 0 ;
	m_eSate = ePlayerState_Online ;
	m_nSessionID = 0 ;
	m_nDisconnectTime = 0 ;
	m_pTimerSave = 0 ;
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		 m_vAllComponents[i] = NULL;
	}
}

CPlayer::~CPlayer()
{
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
			delete p ;
		m_vAllComponents[i] = NULL ;
	}

	if ( m_pTimerSave )
	{
		CGameServerApp::SharedGameServerApp()->GetTimerMgr()->RemoveTimer(m_pTimerSave) ;
		m_pTimerSave = NULL ;
	}
}

void CPlayer::Init(unsigned int nUserUID, unsigned int nSessionID )
{
	m_nSessionID = nSessionID ;
	m_nUserUID = nUserUID ;
	m_eSate = ePlayerState_Online ;
	m_pTimerSave = NULL ;
	m_nDisconnectTime = 0 ;
	/// new components ;here ;
	m_vAllComponents[ePlayerComponent_BaseData] = new CPlayerBaseData(this) ;
	m_vAllComponents[ePlayerComponent_Mail] = new CPlayerMailComponent(this);
	//m_vAllComponents[ePlayerComponent_PlayerItemMgr] = new CPlayerItemComponent(this);
	m_vAllComponents[ePlayerComponent_PlayerMission] = new CPlayerMission(this);
	//m_vAllComponents[ePlayerComponent_PlayerShop] = new CPlayerShop(this);
	m_nTaxasRoomID = 0 ;
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->Init();
		}
	}

	if ( m_pTimerSave == NULL )
	{
		m_pTimerSave = CGameServerApp::SharedGameServerApp()->GetTimerMgr()->AddTimer(this,cc_selector_timer(CPlayer::OnTimerSave)) ;
		m_pTimerSave->SetDelayTime( TIME_SAVE * 0.5 ) ;
		m_pTimerSave->SetInterval(TIME_SAVE) ;
		m_pTimerSave->Start();
	}
}

void CPlayer::Reset(unsigned int nUserUID, unsigned int nSessionID )
{
	m_nDisconnectTime = 0 ;
	m_nSessionID = nSessionID ;
	m_nUserUID = nUserUID ;
	m_nTaxasRoomID = 0 ;
	m_eSate = ePlayerState_Online ;
	// inform components;
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->Reset();
		}
	}

	if ( m_pTimerSave )
	{
		m_pTimerSave->Reset();
		m_pTimerSave->Start();
	}
}

bool CPlayer::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	if ( ProcessPublicPlayerMsg(pMsg,eSenderPort) )
	{
		return true; 
	}

	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			if ( p->OnMessage(pMsg,eSenderPort) )
			{
				return true;
			}
		}
	}

	CLogMgr::SharedLogMgr()->ErrorLog("Unprocessed msg id = %d, from = %d  uid = %d",pMsg->usMsgType,eSenderPort,GetUserUID() ) ;

	return false ;

// 	switch ( pMsg->usMsgType )
// 	{
// 	case MSG_PLAYER_CREATE_PRIVATE_ROOM:
// 		{
// 			//stMsgPlayerCreatePrivateRoomRet msgBack ;
// 			//msgBack.nRet = 0 ;
// 			//msgBack.nPassword = 0;
// 			//msgBack.nRoomID = 0 ;
// 			//if ( GetState() != ePlayerState_Free )
// 			//{
// 			//	msgBack.nRet = 1 ;
// 			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 			//	return ;
// 			//}
// 
// 			//stMsgPlayerCreatePrivateRoom* pMsgRet = (stMsgPlayerCreatePrivateRoom*)pMsg;
// 			//
// 			//if ( (pMsgRet->bDiamond && GetBaseData()->GetAllDiamoned() < pMsgRet->nOwnMoneyNeedToEnter) || (pMsgRet->bDiamond == false && GetBaseData()->GetAllCoin() < pMsgRet->nOwnMoneyNeedToEnter) )
// 			//{
// 			//	msgBack.nRet = 3 ;
// 			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 			//	return ;
// 			//}
// 
// 			//if ( pMsgRet->nBigBinld > pMsgRet->nOwnMoneyNeedToEnter )
// 			//{
// 			//	msgBack.nRet = 4 ;
// 			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 			//	return ;
// 			//}
// 
// 			//CLogMgr::SharedLogMgr()->ErrorLog("do you have create private room card ? " ) ;
// 			//CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)GetComponent(ePlayerComponent_PlayerItemMgr);	
// 			//if ( !pItemMgr->OnUserItem(ITEM_ID_CREATE_ROOM) )
// 			//{
// 			//	msgBack.nRet = 2 ;
// 			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 			//	return ;
// 			//}
// 
// 			//CRoomBase* pBase = CGameServerApp::SharedGameServerApp()->GetRoomMgr()->CreateRoom(eRoom_TexasPoker_Private,eRoomLevel_Junior) ;
// 			//if ( !pBase )
// 			//{
// 			//	CLogMgr::SharedLogMgr()->ErrorLog("create room failed") ;
// 			//	msgBack.nRet = 5 ;
// 			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 			//	pItemMgr->AddItemByID(ITEM_ID_CREATE_ROOM);
// 			//	return ;
// 			//}
// 			//pBase->SetRoomName(pMsgRet->cRoomName); 
// 			//pBase->SetIsDiamonedRoom(pMsgRet->bDiamond) ;
// 			//pBase->SetPassword(pMsgRet->nPassword) ;
// 			//pBase->SetAntesCoin(pMsgRet->nOwnMoneyNeedToEnter) ;
// 			//((CRoomTexasPoker*)pBase)->SetBigBlindBet(pMsgRet->nBigBinld) ;
// 			//msgBack.nPassword = pMsgRet->nPassword ;
// 			//msgBack.nRoomID = pBase->GetRoomID() ;
// 			//SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
// 
// 			//// after crate success , you default enter the room you create ;
// 			//CRoomPeer* peer = (CTaxasPokerPeer*)GetComponent(ePlayerComponent_RoomPeerTaxasPoker) ;
// 			//pBase->AddPeer(peer) ;
// 			//peer->SetRoom(pBase) ;
// 			//m_eSate = ePlayerState_InRoom ;
// 			//pBase->SendCurRoomInfoToPlayer(peer) ;
// 		}
// 		return ;
// 	case MSG_ROOM_ENTER:
// 		{
// 			stMsgRoomEnterRet msgRet ;
// 			msgRet.nRet = 0 ;
// 			if ( m_eSate == ePlayerState_InRoom )
// 			{
// 				msgRet.nRet = 3 ;   // aready in room ;
// 				SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
// 				return ;
// 			}
// 
// 			stMsgRoomEnter* msgEnter = (stMsgRoomEnter*)pMsg ;
// 			CRoomBaseNew* pRoom = CGameServerApp::SharedGameServerApp()->GetRoomMgr()->GetRoom(msgEnter->nRoomType,msgEnter->nRoomLevel,msgEnter->nRoomID) ; 
// 			if ( pRoom == NULL )
// 			{
// 				pRoom = CGameServerApp::SharedGameServerApp()->GetRoomMgr()->CreateRoom(msgEnter->nRoomType,msgEnter->nRoomLevel);
// 			}
// 
// 			if ( pRoom == NULL )
// 			{
// 				msgRet.nRet = 6 ;   // room is NULL;
// 				SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
// 				return ;
// 			}
// 
// 			unsigned char nRet = pRoom->CheckCanJoinThisRoom(this);
// 			if ( nRet != 0 )
// 			{
// 				msgRet.nRet = 5 ;   // room is NULL;
// 				SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
// 				return ;
// 			}
// 			SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
// 			pRoom->Enter(this);
// 			m_pCurRoom = pRoom ;
// 			m_eSate = ePlayerState_InRoom ;
// 		}
// 		return ;
// 	default:
// 		break; 
// 	}

}

void CPlayer::OnPlayerDisconnect()
{
	// inform components;
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->OnPlayerDisconnect();
		}
	}

	if ( m_pTimerSave )
	{
		m_pTimerSave->Stop();
	}
	
	m_nDisconnectTime = time(NULL) ;
	SetState(ePlayerState_Offline) ;

	if ( m_nTaxasRoomID != 0 )  // order to leave 
	{
		stMsgOrderTaxasPlayerLeave msgLeave ;
		msgLeave.nRoomID = m_nTaxasRoomID ;
		CGameServerApp::SharedGameServerApp()->SendMsg(GetSessionID(),(char*)&msgLeave,sizeof(msgLeave) ) ;
	}
}

void CPlayer::PostPlayerEvent(stPlayerEvetArg* pEventArg )
{
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->OnPlayerEvent(pEventArg);
		}
	}
}

void CPlayer::SendMsgToClient(const char* pBuffer, unsigned short nLen,bool bBrocat )
{
	if ( IsState(ePlayerState_Online)  )
	{
		CGameServerApp::SharedGameServerApp()->SendMsg(GetSessionID(),pBuffer,nLen,bBrocat) ;
		return ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("player uid = %d not online so , can not send msg" ,GetUserUID() ) ;
}

bool CPlayer::IsState( ePlayerState eState )
{
	if ( (m_eSate & eState) == eState )
	{
		return true ;
	}
	return false ;
}

void CPlayer::OnAnotherClientLoginThisPeer(unsigned int nSessionID )
{
	// tell prelogin client to disconnect ;
	stMsgPlayerOtherLogin msg ;
	SendMsgToClient((char*)&msg,sizeof(msg)) ;

	if ( m_nTaxasRoomID != 0 )  // order to leave 
	{
		stMsgOrderTaxasPlayerLeave msgLeave ;
		msgLeave.nRoomID = m_nTaxasRoomID ;
		CGameServerApp::SharedGameServerApp()->SendMsg(GetSessionID(),(char*)&msgLeave,sizeof(msgLeave) ) ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("pls remember inform other server this envent OnAnotherClientLoginThisPeer ") ;

	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->OnOtherWillLogined();
		}
	}
	// bind new client ;
	m_nSessionID = nSessionID ;
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->OnOtherDoLogined();
		}
	}
}

bool CPlayer::ProcessPublicPlayerMsg(stMsg* pMsg , eMsgPort eSenderPort)
{
	switch ( pMsg->usMsgType )
	{
// 	case MSG_ROBOT_INFORM_IDLE:
// 		{
// 			CRobotManager::SharedRobotMgr()->AddIdleRobotPlayer(this) ;
// 		}
// 		break;
// 	case MSG_ROBOT_ADD_MONEY:
// 		{
// 			stMsgRobotAddMoney* pAdd = (stMsgRobotAddMoney*)pMsg ;
// 			GetBaseData()->ModifyMoney(pAdd->nWantCoin) ;
// 			stMsgRobotAddMoneyRet msgBack ;
// 			msgBack.cRet = 0 ;
// 			msgBack.nFinalCoin = GetBaseData()->GetAllCoin();
// 			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 		}
// 		break;
// 	case MSG_REQUEST_RANK:
// 		{
// 			stMsgPlayerRequestRank* pMsgRet = (stMsgPlayerRequestRank*)pMsg ;
// 			CGameServerApp::SharedGameServerApp()->GetGameRanker()->SendRankToPlayer(this,(eRankType)pMsgRet->nRankType,pMsgRet->nFromIdx,pMsgRet->nCount ) ;
// 		}
// 		break;
// 	case MSG_REQUEST_RANK_PEER_DETAIL:
// 		{
// 			stMsgPlayerRequestRankPeerDetail* pRetMsg = (stMsgPlayerRequestRankPeerDetail*)pMsg ;
// 			CGameServerApp::SharedGameServerApp()->GetGameRanker()->SendRankDetailToPlayer(this,pRetMsg->nRankPeerUID,(eRankType)pRetMsg->nRankType);
// 		}
// 		break;
// 	case MSG_PLAYER_SAY_BROCAST:
// 		{
// 			stMsgPlayerSayBrocast* pMsgRet = (stMsgPlayerSayBrocast*)pMsg ;
// 			CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)GetComponent(ePlayerComponent_PlayerItemMgr);	
// 			stMsgPlayerSayBrocastRet msg ;
// 			msg.nRet = 0 ;
// 			if ( pItemMgr->OnUserItem(ITEM_ID_LA_BA) )
// 			{
// 				CGameServerApp::SharedGameServerApp()->GetBrocaster()->PostPlayerSayMsg(this,((char*)pMsg) + sizeof(stMsgPlayerSayBrocast),pMsgRet->nContentLen) ;
// 			}
// 			else
// 			{
// 				msg.nRet = 1 ;
// 				CLogMgr::SharedLogMgr()->ErrorLog(" you have no la ba") ;
// 			}
// 			SendMsgToClient((char*)&msg,sizeof(msg)) ;
// 		}
// 		break;
// 	case MSG_PLAYER_REPLAY_BE_INVITED:
// 		{
// 			//stMsgPlayerRecievedInviteReply toMsgInviter; // who invite me ;
// 			stMsgPlayerReplayBeInvitedToJoinRoom* pMsgRet = (stMsgPlayerReplayBeInvitedToJoinRoom*)pMsg ;
// 			//toMsgInviter.nRet = 0 ;
// 			stMsgPlayerReplayBeInvitedToJoinRoomRet msgBack ;
// 			if ( pMsgRet->nReplyResult == 1 ) // refused 
// 			{
// 				//toMsgInviter.nRet = 1 ;
// 				msgBack.nRet = 0 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 			else  // i agreed ;
// 			{
// 				msgBack.nRet = 0 ;
// 				CTaxasPokerPeer* pThisPeer = (CTaxasPokerPeer*)GetComponent(ePlayerComponent_RoomPeerTaxasPoker);
// 				CRoomTexasPoker* pRoomToEnter = (CRoomTexasPoker*)CGameServerApp::SharedGameServerApp()->GetRoomMgr()->GetRoom(pMsgRet->nRoomType,pMsgRet->nRoomLevel,pMsgRet->nRoomID) ;
// 				if ( !pRoomToEnter || pRoomToEnter->CanPeerSitDown(pThisPeer) == false )
// 				{
// 					//toMsgInviter.nRet = 4 ;
// 					msgBack.nRet = 2 ;
// 				}
// 				else
// 				{
// 					// join room ;
// 					pThisPeer->LeaveRoom();
// 					if ( pRoomToEnter->AddBeInvitedPlayer(this,pMsgRet->nSitIdx) == false )
// 					{
// 						//toMsgInviter.nRet = 3 ;
// 						msgBack.nRet = 3 ;
// 					}
// 				}
// 
// 				if ( msgBack.nRet != 0 )  // only failed , tell client , when success , directly enter room ;
// 				{
// 					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				}
// 			}
// 			//CPlayer* pToInvid = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nReplyToUserUID) ;
// 			//if ( pToInvid && toMsgInviter.nRet != 0 ) // only failed situation ,tell inviter ;
// 			//{
// 			//	memcpy(toMsgInviter.nReplyerName,GetBaseData()->GetPlayerName(),MAX_LEN_CHARACTER_NAME);
// 			//	pToInvid->SendMsgToClient((char*)&toMsgInviter,sizeof(toMsgInviter)) ;
// 			//}
// 			//else
// 			//{
// 			//	CLogMgr::SharedLogMgr()->PrintLog("the one who invite me had offline , his uid = %d",pMsgRet->nReplyToUserUID) ;
// 			//}
// 		}
// 		break;
// 	case MSG_PLAYER_FOLLOW_TO_ROOM:
// 		{
// 			stMsgPlayerFollowToRoom* pRetMsg = (stMsgPlayerFollowToRoom*)pMsg ;
// 			CPlayer* pTargetPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pRetMsg->nTargetPlayerUID) ;
// 			stMsgPlayerFollowToRoomRet msgBack ;
// 			msgBack.nRet = 0 ;
// 			if ( pTargetPlayer == NULL )
// 			{
// 				msgBack.nRet = 1 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break ;
// 			}
// 
// 			if ( ePlayerState_Free == pTargetPlayer->GetState() )
// 			{
// 				msgBack.nRet = 2 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break ;
// 			}
// 
// 			if ( ePlayerState_Free != GetState() )
// 			{
// 				msgBack.nRet = 4 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break ;
// 			}
// 
// 			CRoomBaseNew* pStateRoom = pTargetPlayer->GetRoomCurStateIn() ;
// 			if ( !pStateRoom )
// 			{
// 				CLogMgr::SharedLogMgr()->ErrorLog("follow to a null room , but target player is not free , how , why ?") ;
// 				msgBack.nRet = 2 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 
// 			if ( pStateRoom->CheckCanJoinThisRoom(pTargetPlayer) != 0)
// 			{
// 				msgBack.nRet = 3 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 			// add to room 
// 			stMsgRoomEnter msgToEnterRoom ;
// 			msgToEnterRoom.nPassword = 0 ;
// 			msgToEnterRoom.nRoomID = pStateRoom->GetRoomID();
// 			msgToEnterRoom.nRoomLevel = pStateRoom->GetRoomLevel() ;
// 			msgToEnterRoom.nRoomType = pStateRoom->GetRoomType() ;
// 			OnMessage(&msgToEnterRoom);
// 		}
// 		break;
// 	case MSG_PLAYER_SLOT_MACHINE:
// 		{
// 			stMsgPlayerSlotMachineRet msgBack ;
// 			msgBack.nRet = 0 ;
// 			stMsgPlayerSlotMachine* pRetMsg = (stMsgPlayerSlotMachine*)pMsg ;
// 			if ( GetBaseData()->GetAllCoin() < pRetMsg->nBetCoin || GetBaseData()->GetAllDiamoned() < pRetMsg->nBetDiamoned )
// 			{
// 				msgBack.nRet = 1 ;
// 			}
// 			else
// 			{
// 				CSlotMachine* pMachine = (CSlotMachine*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_SlotMachine) ;
// 				float fRate = 0 ;
// 				pMachine->RandSlotMachine(pRetMsg->cLevel,msgBack.vCard,fRate) ;
// 				int64_t nOffsetCoin = pRetMsg->nBetCoin * ( fRate - 1.0 );
// 				int nOffsetDiamoned = pRetMsg->nBetDiamoned * ( fRate - 1.0 );
// 				nOffsetDiamoned = abs(nOffsetDiamoned) ;
// 				nOffsetCoin = abs(nOffsetCoin) ;
// 				int nOffset = fRate > 1 ? 1 : -1 ;
// 				GetBaseData()->ModifyMoney(nOffsetCoin * nOffset);
// 				GetBaseData()->ModifyMoney(nOffsetDiamoned * nOffset,true);				
// 				msgBack.nFinalAllCoin = GetBaseData()->GetAllCoin();
// 				msgBack.nFinalDiamoned = GetBaseData()->GetAllDiamoned() ;
// 				msgBack.nTakeInCoin = GetBaseData()->GetTakeInMoney() ;
// 				msgBack.nTakeInDiamoned = GetBaseData()->GetTakeInMoney(true) ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 			}
// 		}
// 		break;
// 	case MSG_ROOM_REQUEST_PEER_DETAIL:
// 		{
// 			stMsgRoomRequestPeerDetailRet msgBack ;
// 			stMsgRoomRequestPeerDetail* pMsgRet = (stMsgRoomRequestPeerDetail*)pMsg ;
// 			msgBack.nPeerSessionID = pMsgRet->nPeerSessionID ;
// 			CPlayer* pDetailPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(pMsgRet->nPeerSessionID) ;
// 			if ( pDetailPlayer == NULL )
// 			{
// 				msgBack.nRet = 1 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				return true ;
// 			}
// 
// 			pDetailPlayer->GetBaseData()->GetPlayerDetailData(&msgBack.stDetailInfo);
// 			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 		}
// 		break;
 	default:
 		return false ;
	}
	return true ;
}

void CPlayer::OnTimerSave(float fTimeElaps,unsigned int nTimerID )
{
	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->TimerSave();
		}
	}
}

void CPlayer::PushTestAPNs()
{
#ifdef NDEBUG
	return ;
#endif
	//if ( GetBaseData()->bPlayerEnableAPNs == false )
	//{
	//	CLogMgr::SharedLogMgr()->ErrorLog("you not enable apns ") ;
	//	return ;
	//}
	//char* pString = "\"you disconnected \"" ;
	//stMsgToAPNSServer msg ;
	//msg.nAlertLen = strlen(pString) ;
	//msg.nBadge = 1 ;
	//msg.nSoundLen = 0 ;
	//memcpy(msg.pDeveiceToken,GetBaseData()->vAPNSToken,32);
	//char* pBuffer = new char[sizeof(msg) + msg.nAlertLen ] ;
	//unsigned short nOffset = 0 ;
	//memcpy(pBuffer,&msg,sizeof(msg));
	//nOffset += sizeof(msg);
	//memcpy(pBuffer + nOffset , pString ,msg.nAlertLen);
	//nOffset += msg.nAlertLen ;
	//CGameServerApp::SharedGameServerApp()->SendMsgToAPNsServer(pBuffer,nOffset);
	//delete[] pBuffer ;
}

void CPlayer::OnReactive(uint32_t nSessionID )
{
	m_nSessionID = nSessionID ;
	SetState(ePlayerState_Online) ;
	m_nDisconnectTime = 0 ;
	
	if ( m_pTimerSave )
	{
		m_pTimerSave->Reset();
		m_pTimerSave->Start();
	}


	for ( int i = ePlayerComponent_None; i < ePlayerComponent_Max ; ++i )
	{
		IPlayerComponent* p = m_vAllComponents[i] ;
		if ( p )
		{
			p->OnReactive(nSessionID);
		}
	}
}
