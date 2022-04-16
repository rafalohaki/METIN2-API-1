/*
* blackdragonx61 / Mali
* 12.04.2022
*/

#ifndef __M2API_H__
#define __M2API_H__

#include <winsock2.h>
#include <windows.h>
#include <memory>
#include <vector>
#include <cassert>

namespace M2API
{
#pragma pack(1)
	typedef struct packet_phase
	{
		BYTE	header;
		BYTE	phase;
	} TPacketGCPhase;

	typedef struct packet_handshake
	{
		BYTE	bHeader;
		DWORD	dwHandshake;
		DWORD	dwTime;
		long	lDelta;
	} TPacketGCHandshake;

	typedef struct SChannelStatus
	{
		short nPort;
		BYTE bStatus;
	} TChannelStatus;
#pragma pack()

	enum class PACKET_SERVER
	{
		HEADER_CG_TEXT = 64,
		HEADER_CG_MARK_LOGIN = 100,
		HEADER_CG_STATE_CHECKER = 206,
		HEADER_CG_KEY_AGREEMENT = 0xfb, // _IMPROVED_PACKET_ENCRYPTION_
		HEADER_CG_PONG = 0xfe,
		HEADER_CG_HANDSHAKE = 0xff,
	};

	class CBuffer
	{
	public:
		CBuffer(size_t size) :
			m_BufferVec(size),
			m_Offset(0)
		{
		}

		~CBuffer()
		{
			// assert(m_Offset == m_BufferVec.size());
		};

		using BUFFER_VEC = std::vector<char>;
		BUFFER_VEC::value_type* GetData() { return m_BufferVec.data(); }

		size_t GetSize() const { return m_BufferVec.size(); }
		void Resize(size_t size) { return m_BufferVec.resize(size); }

		template<typename T, typename std::enable_if<!std::is_same<T, const char*>::value>::type* = nullptr>
		T Decode()
		{
			assert(m_BufferVec.size() >= m_Offset + sizeof(T));
			
			T value = *reinterpret_cast<T*>(m_BufferVec.data() + m_Offset);
			m_Offset += sizeof(T);

			return value;
		}

		template <>
		std::string Decode<std::string>()
		{
			assert(m_BufferVec.size() > m_Offset);
			
			std::string value(reinterpret_cast<const char*>(m_BufferVec.data() + m_Offset));
			m_Offset += value.size();

			if (value.empty() == false && value.back() == '\n')
				value.pop_back();

			return value;
		}

	public:
		size_t m_Offset;

	private:
		BUFFER_VEC m_BufferVec;
	};

	class CM2API
	{
	public:
		explicit CM2API(const char* pw, const char* ip, WORD port) :
			m_Password(pw),
			m_IP(ip),
			m_Port(port),
			m_SocketFd(INVALID_SOCKET),
			m_WSA(std::make_unique<WSADATA>()),
			m_ConnectResult(SOCKET_ERROR)
		{
			if (WSAStartup(MAKEWORD(2, 2), m_WSA.get()) != ERROR_SUCCESS) {
				printf("WSAStartup Failed. Error Code: %d\n", WSAGetLastError());
				m_WSA = nullptr;
				return;
			}

			m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
			if (m_SocketFd == INVALID_SOCKET) {
				printf("Can't create socket! Error Code: %d\n", WSAGetLastError());
				return;
			}

			struct sockaddr_in server;
			server.sin_addr.s_addr = inet_addr(GetIP().c_str());
			server.sin_family = AF_INET;
			server.sin_port = htons(GetPort());

			m_ConnectResult = connect(m_SocketFd, (struct sockaddr*)&server, sizeof(server));
			if (m_ConnectResult == SOCKET_ERROR) {
				printf("Connect fail! Error Code: %d\n", WSAGetLastError());
				return;
			}
		}

		virtual ~CM2API()
		{
			if (m_SocketFd != INVALID_SOCKET) {
				closesocket(m_SocketFd);
			}

			if (m_WSA) {
				WSACleanup();
			}
		}

		CBuffer* SendAndGet(size_t size, PACKET_SERVER p, const std::string& msg = "")
		{
			if (m_ConnectResult == SOCKET_ERROR)
				return nullptr;

			std::string message = (char)p + msg;
			if (p == PACKET_SERVER::HEADER_CG_TEXT)
				message += '\n';

			int iResult = send(m_SocketFd, message.c_str(), static_cast<int>(message.size()), 0);
			if (iResult == SOCKET_ERROR) {
				printf("Send Failed! Error Code: %d\n", WSAGetLastError());
				return nullptr;
			}

			CBuffer* pBuffer = new CBuffer(size);

			iResult = recv(m_SocketFd, pBuffer->GetData(), static_cast<int>(pBuffer->GetSize()), 0);
			if (iResult == SOCKET_ERROR) {
				printf("Recv Failed! Error Code: %d\n", WSAGetLastError());
				delete pBuffer;
				return nullptr;
			}

			pBuffer->Resize(iResult);
			return pBuffer;
		}

		const std::string& GetPassword() const { return m_Password; }
		const std::string& GetIP() const { return m_IP; }
		WORD GetPort() const { return m_Port; }

	private:
		std::string m_Password;
		std::string m_IP;
		WORD m_Port;
		SOCKET m_SocketFd;
		std::unique_ptr<WSADATA> m_WSA;
		int m_ConnectResult;
	};
}

#endif