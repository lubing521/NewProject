#include "LoginScene.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PaiJiuMessageDefine.h"
#include "PaiJiuScene.h"
#include "ClientRobot.h"
#include "TaxasPokerScene.h"
#include "BacScene.h"
CLoginScene::CLoginScene(CClientRobot* pNetWork ):IScene(pNetWork)
{ 
	m_eSceneType = eScene_Login ;
}

void CLoginScene::OnEnterScene()
{
	IScene::OnEnterScene();
	//InformIdle();
}

void CLoginScene::OnEixtScene()
{
	IScene::OnEixtScene();
}

bool CLoginScene::OnMessage( Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	IScene::OnMessage(pPacket) ;
	switch (pMsg->usMsgType)
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegisterRet* pRet = (stMsgRegisterRet*)pMsg ;
			if ( pRet->nRet == 0 )
			{
				CLogMgr::SharedLogMgr()->SystemLog("register sucess accout = %s , password = %s,uid = %d",pRet->cAccount,pRet->cPassword ,pRet->nUserID ) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("register error code = %d",pRet->nRet );
			}
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet* pRetMsg = (stMsgLoginRet*)pMsg;
			 // 0 ; success ; 1 account error , 2 password error ;
			const char* pString = NULL ;
			if ( pRetMsg->nRet == 0 )
			{
				pString = "check Account Success !" ;
			}
			else if ( 1 == pRetMsg->nRet )
			{
				pString = "Check Account Error : Account Error !" ;
			}
			else if ( 2 == pRetMsg->nRet )
			{
				pString = "Check Account Error : password Error !" ;
			}
			else
			{
				pString = "Check account Error : unknown Error !" ;
			}

			CLogMgr::SharedLogMgr()->SystemLog("%s ret = %d, account type = %d ",pString,pRetMsg->nRet,pRetMsg->nAccountType ) ;

// 			stMsgModifyPassword msg ;
// 			msg.nUserUID = 1201;
// 			memset(msg.cNewPassword,0,sizeof(msg.cNewPassword)) ;
// 			memset(msg.cOldPassword,0,sizeof(msg.cOldPassword)) ;
// 			sprintf_s(msg.cNewPassword,"12") ;
// 			sprintf_s(msg.cOldPassword,"6") ;
// 			SendMsg(&msg,sizeof(msg)) ;

			stMsgRebindAccount msg ;
			msg.nCurUserUID = 1198 ;
			memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
			memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
			sprintf_s(msg.cAccount,"2d3") ;
			sprintf_s(msg.cPassword,"6") ;
			SendMsg(&msg,sizeof(msg)) ;
			return true ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccountRet* pRet = (stMsgRebindAccountRet*)pMsg ;
			CLogMgr::SharedLogMgr()->SystemLog("bind account ret = %d",pRet->nRet ) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPasswordRet* pRet = (stMsgModifyPasswordRet*)pMsg ;
			CLogMgr::SharedLogMgr()->SystemLog("modify password ret = %d ",pRet->nRet);
		}
		break;
	case MSG_ROBOT_ORDER_TO_ENTER_ROOM:
		{
			stMsgRobotOrderToEnterRoom* pOrder = (stMsgRobotOrderToEnterRoom*)pMsg;

			stMsgRoomEnter msgEnterRoom ;
			msgEnterRoom.nRoomID = pOrder->nRoomID;
			msgEnterRoom.nRoomType = pOrder->nRoomType;
			msgEnterRoom.nRoomLevel = pOrder->cLevel;
			SendMsg(&msgEnterRoom, sizeof(msgEnterRoom) ) ;
			printf("order robot to enter type = %d , room level = %d , room id = %d \n",pOrder->nRoomType,pOrder->cLevel,pOrder->nRoomID);
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			// get room list ;
			//stMsgRequestRoomList msgRoomList ;
			//msgRoomList.cRoomType = m_pClient->GetPlayerData()->GetEnterRoomType() ;
			//SendMsg(&msgRoomList,sizeof(msgRoomList));
			InformIdle();
		}
		break; ;
	//case MSG_REQUEST_ROOM_LIST:
	//	{
	//		stMsgRequestRoomListRet* Ret = (stMsgRequestRoomListRet*)pMsg ;
	//		stRoomListItem* pRoomList = (stRoomListItem*)(((char*)Ret) + sizeof(stMsgRequestRoomListRet)) ;
	//		// --target room less in all 
	//		stRoomListItem* pLestAll = NULL ;
	//		unsigned short nAllLeftSeat = 0 ;
	//		//-------target in single level ;
	//		stRoomListItem* pLestInLevel = NULL ;
	//		unsigned short nSingleLevelLeft = 0 ;
	//		// specail room 
	//		stRoomListItem* pSecailRoom = NULL ;

	//		CLogMgr::SharedLogMgr()->PrintLog("%s: RecieveRoomList Type = %d", m_pClient->GetPlayerData()->GetName(),Ret->cRoomType) ;
	//		while ( Ret->nRoomCount--)
	//		{
	//			// Lest all ;
	//			if ( pRoomList->nMaxCount - pRoomList->nCurrentCount > nAllLeftSeat )
	//			{
	//				pLestAll = pRoomList ;
	//				nAllLeftSeat = pRoomList->nMaxCount - pRoomList->nCurrentCount ;
	//			}

	//			// in lest level 
	//			if ( pRoomList->cRoomLevel == m_pClient->GetPlayerData()->GetWillEnterRoomLevel() )
	//			{
	//				if ( pRoomList->nMaxCount - pRoomList->nCurrentCount > nSingleLevelLeft )
	//				{
	//					pLestInLevel = pRoomList ;
	//					nSingleLevelLeft = pRoomList->nMaxCount - pRoomList->nCurrentCount ;
	//				}

	//				if ( pRoomList->nRoomID == m_pClient->GetPlayerData()->GetEnterRoomID() )
	//				{
	//					pSecailRoom = pRoomList ;
	//				}
	//			}
	//			printf("%s: RoomID = %d , RoomLevel=%d , MaxCount =%d  , CurrentCount = %d \n", m_pClient->GetPlayerData()->GetName(), pRoomList->nRoomID,pRoomList->cRoomLevel,pRoomList->nMaxCount,pRoomList->nCurrentCount) ;
	//			++pRoomList ;
	//		}

	//		if ( pLestAll == 0 && pLestInLevel == 0 && pSecailRoom == 0 )
	//		{
	//			printf("%s: Can not Enter config Room ID = %d , RoomType = %d", m_pClient->GetPlayerData()->GetName(),m_pClient->GetPlayerData()->GetEnterRoomID(),m_pClient->GetPlayerData()->GetEnterRoomID()) ;	
	//			m_pClient->Stop();
	//			return true ;
	//		}

	//		stRoomListItem* pFinal = NULL ;
	//		if ( m_pClient->GetPlayerData()->GetWillEnterRoomLevel() < 0 )
	//		{
	//			pFinal = pLestAll ;
	//		}
	//		else if ( m_pClient->GetPlayerData()->GetEnterRoomID() <= 0 )
	//		{
	//			pFinal = pLestInLevel ;
	//		}
	//		else
	//		{
	//			pFinal = pSecailRoom ;
	//		}

	//		stMsgRoomEnter msgEnterRoom ;
	//		msgEnterRoom.nRoomID = 0;
	//		//msgEnterRoom.nRoomType = m_pClient->GetPlayerData()->GetEnterRoomType() ;
	//		//msgEnterRoom.nRoomLevel = pFinal->cRoomLevel ;
	//		SendMsg(&msgEnterRoom, sizeof(msgEnterRoom) ) ;
	//		return true ;
	//	}
	//	break; 
	case MSG_ROOM_ENTER:
		{
			// 0 success ; 1 do not meet room condition , 2 aready in room ; 3  unknown error ; 4 waiting last game settlement ;
			stMsgRoomEnterRet* pRetMsg = (stMsgRoomEnterRet*)pMsg ;
			switch ( pRetMsg->nRet )
			{
			case 0:
				{
					CLogMgr::SharedLogMgr()->SystemLog("%s EnterRoom Success !",m_pClient->GetPlayerData()->GetName()) ;
				}
				break; 
			case 1 :
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s do not meet room condition !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break ;
			case 2:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s already in room !",m_pClient->GetPlayerData()->GetName()) ;
				}
				break; 
			case 3:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s unknown error !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break; 
			case 4:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s waiting last game settlement !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break;
			default:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s Default error !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break;
			}
		}
		break;
	case MSG_TP_ROOM_BASE_INFO:
		{
			// change room scene and push this msg;
			IScene*pScene = new CTaxasPokerScene(m_pClient) ;
			m_pClient->ChangeScene(pScene) ;
			pScene->OnMessage(pPacket) ;
			return true ;
		}
		break;
	case MSG_ROBOT_ADD_MONEY:
		{
			stMsgRobotAddMoneyRet* pRet = (stMsgRobotAddMoneyRet*)pMsg ;
			m_pClient->GetPlayerData()->nMyCoin = pRet->nFinalCoin ;
			printf("received add coin !\n") ;
			stMsgRobotInformIdle msg ;
			SendMsg((char*)&msg,sizeof(msg)) ;
			return true;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->SystemLog("%s Unknown message CLoginScene msg = %d!",m_pClient->GetPlayerData()->GetName(),pMsg->usMsgType ) ;
		}
		break;
	}
	return false ;
}

void CLoginScene::Verifyed()
{
	Register("hello name","23s","6",1);
	//Login("23","6");
}

void CLoginScene::Login( const char* pAccound , const char* pPassword )
{
	stMsgLogin msg ;
	memset(msg.cAccount,0,sizeof(msg.cAccount));
	memset(msg.cPassword,0,sizeof(msg.cPassword));
	sprintf_s(msg.cAccount,"%s",pAccound);
	sprintf_s(msg.cPassword,"%s",pPassword) ;
	SendMsg((char*)&msg,sizeof(msg)) ;
}

void CLoginScene::InformIdle()
{
	if ( m_pClient->GetPlayerData()->GetCoin(false) < 200000 )
	{
		stMsgRobotAddMoney msg ;
		msg.nWantCoin = 5500000 ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
	else
	{
		stMsgRobotInformIdle msg ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
}

void CLoginScene::Register( const char* pName ,const char* pAccound , const char* pPassword , int nType )
{
	stMsgRegister msg ;
	msg.cRegisterType = nType ;
	msg.nChannel = 0 ;
	memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
	memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
	if ( nType != 0 )
	{
		sprintf_s(msg.cAccount,"%s",pAccound) ;
		sprintf_s(msg.cPassword,"%s",pPassword) ;
	}
	m_pClient->GetNetWork()->SendMsg((char*)&msg,sizeof(msg)) ;
}