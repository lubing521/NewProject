#pragma once
#include "Timer.h"
#include <map>
class CPlayer ;
struct stMsg ;
class CRoomBaseData;
struct stRoomBaseDataOnly ;
struct stBaseRoomConfig ;

class CRoomBaseNew
	:public CTimerDelegate
{
public:
	typedef std::map<unsigned int , CPlayer*> MAP_SESSION_PLAYER ;
public:
	CRoomBaseNew();
	virtual ~CRoomBaseNew();
	virtual bool Init(stBaseRoomConfig* pConfig);
	virtual void Enter(CPlayer* pEnter );
	virtual void Leave(CPlayer* pLeaver);
	unsigned int GetRoomID();
	unsigned char GetRoomType();
	unsigned char GetRoomLevel();
	virtual void Update(float fTimeElpas, unsigned int nTimerID ) ;
	virtual bool OnMessage(CPlayer*pSender, stMsg* pmsg);
	void SendMsgBySessionID(stMsg* pMsg , unsigned short nLen ,unsigned int nSessionID = 0,bool bToAll = true );  // if bToAll = true , means send all peers in this room, nSessionId is except ;
	void SendMsgByRoomIdx(stMsg* pMg, unsigned short nLen ,unsigned char nIdx);
	virtual void SitDown(CPlayer* pPlayer );
	virtual void StandUp(CPlayer* pPlayer) ;
	virtual unsigned char CheckCanJoinThisRoom(CPlayer* pPlayer); // 0 means ok , other value means failed ;
	virtual void SendRoomInfoToPlayer(CPlayer* pPlayer);
	CPlayer* GetPlayerByRoomIdx(unsigned char pIdx );
	CRoomBaseData* GetData(){ return m_pRoomData;}
	unsigned int GetAntesCoin();
	unsigned short GetEmptySeatCount();
	unsigned short GetMaxSeat();
/*protected:*/
	stRoomBaseDataOnly* GetRoomDataOnly();
protected:
	MAP_SESSION_PLAYER m_vAllPeers; // both playing and seeing ;
	CRoomBaseData* m_pRoomData ;
};