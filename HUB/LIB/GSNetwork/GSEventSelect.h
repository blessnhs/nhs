#pragma once

namespace GSNetwork { namespace GSEventSelect	{

/// @brief WSAEventSelect 기반 소켓 이벤트를 감시하고 파생 클래스의 콜백으로 전달합니다.
class GSEventSelect
{
public:
	/// @brief 소켓 이벤트 선택 객체의 초기 상태를 설정합니다.
	GSEventSelect(void);
	/// @brief 소켓 이벤트 선택 객체를 파괴합니다.
	virtual ~GSEventSelect(void);

private:
	HANDLE			m_SelectEventHandle;
	HANDLE			m_StartupEventHandle;
	HANDLE			m_DestroyEventHandle;

	HANDLE			m_SelectThreadHandle;

	SOCKET			m_Socket;

protected:
	/// @brief FD_READ에 대한 파생 클래스의 수신 콜백입니다.
	virtual VOID	OnIoRead(VOID)			= 0;
	//virtual VOID	OnIoWrote(VOID)			= 0;
	/// @brief FD_CONNECT에 대한 파생 클래스의 연결 콜백입니다.
	virtual VOID	OnIoConnected(VOID)		= 0;
	/// @brief FD_CLOSE에 대한 파생 클래스의 종료 콜백입니다.
	virtual VOID	OnIoDisconnected(VOID)	= 0;

public:
	/// @brief 소켓 이벤트를 등록하고 감시 스레드를 시작합니다.
	BOOL			Initialize(SOCKET socket);
	/// @brief 감시 스레드와 소켓 이벤트 핸들을 정리합니다.
	BOOL			Termination(VOID);

	/// @brief 소켓 이벤트를 대기하고 발생한 이벤트를 대응 콜백으로 전달합니다.
	VOID			SelectThreadCallback(VOID);
};

}	}	