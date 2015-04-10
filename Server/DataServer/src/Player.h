#pragma once 
#include "IPlayerComponent.h"
#include "Timer.h"
#include "ServerMessageDefine.h"
class CPlayerBaseData ;
struct stMsg ;
class CRoomBaseNew ;
class CPlayer
	:public CTimerDelegate
{
public:
	enum ePlayerState
	{
		ePlayerState_Offline = 0 ,
		ePlayerState_Online = 1,
		ePlayerState_InTaxasRoom = (1<<1)| ePlayerState_Online,
		ePlayerState_Max,
	};
public:
	CPlayer();
	~CPlayer();
	void Init( unsigned int nUserUID,unsigned int nSessionID );
	void Reset(unsigned int nUserUID,unsigned int nSessionID ) ; // for reuse the object ;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	void OnPlayerDisconnect();
	void SendMsgToClient(const char* pBuffer, unsigned short nLen,bool bBrocat = false );
	unsigned int GetUserUID(){ return m_nUserUID ;}
	unsigned int GetSessionID(){ return m_nSessionID ;}
	IPlayerComponent* GetComponent(ePlayerComponentType eType ){ return m_vAllComponents[eType];}
	CPlayerBaseData* GetBaseData(){ return (CPlayerBaseData*)GetComponent(ePlayerComponent_BaseData);}
	bool IsState( ePlayerState eState ); 
	void SetState(ePlayerState eSate ){ m_eSate = eSate ; }
	void OnAnotherClientLoginThisPeer(unsigned int nSessionID );
	void PostPlayerEvent(stPlayerEvetArg* pEventArg );
	void OnTimerSave(float fTimeElaps,unsigned int nTimerID );
	void OnReactive(uint32_t nSessionID );
	time_t GetDisconnectTime(){ return m_nDisconnectTime ;}
	void SetStayInTaxasRoomID(uint32_t nRoomID ){ m_nTaxasRoomID = nRoomID ; }
	uint32_t GetTaxasRoomID(){ return m_nTaxasRoomID ; }
protected:
	bool ProcessPublicPlayerMsg( stMsg* pMessage , eMsgPort eSenderPort );
	void PushTestAPNs();
protected:
	unsigned int m_nUserUID ;
	unsigned int m_nSessionID ;  // comunicate with the client ;
	IPlayerComponent* m_vAllComponents[ePlayerComponent_Max] ;
	ePlayerState m_eSate ;
	CTimer* m_pTimerSave ;
	time_t m_nDisconnectTime ;
	uint32_t m_nTaxasRoomID ;
}; 