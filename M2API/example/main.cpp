/*
* blackdragonx61 / Mali
* 12.04.2022
*/

#include <iostream>
#include <M2API.hpp>

int main()
{
	using namespace M2API;

	CM2API p("SHOWMETHEMONEY", "127.0.0.1", 13061);

	try
	{
		// Admin Login
		{
			std::unique_ptr<CBuffer> buffer(p.SendAndGet(64, PACKET_SERVER::HEADER_CG_TEXT, p.GetPassword()));

			if (buffer) {
				TPacketGCPhase phase = buffer->Decode<TPacketGCPhase>();
				TPacketGCHandshake handshake = buffer->Decode<TPacketGCHandshake>();

				//buffer->m_Offset += (sizeof(TPacketGCPhase) + sizeof(TPacketGCHandshake)); // for first connection

				std::string resText = buffer->Decode<std::string>();

				if (!resText.compare("UNKNOWN"))
					printf("ADMIN LOGIN SUCCESS.\n");
				else
					printf("ADMIN LOGIN FAILED: %s\n", resText.c_str());
			}
			else {
				printf("[Admin Login]Cannot get data.\n");
			}
		}

		// USER_COUNT
		{
		    std::unique_ptr<CBuffer> buffer(p.SendAndGet(32, PACKET_SERVER::HEADER_CG_TEXT, "USER_COUNT"));
		
		    if (buffer) {
		        std::string resText = buffer->Decode<std::string>();
		        printf("USER_COUNT Result: %s\n", resText.c_str());
		    } else {
		        printf("[USER_COUNT] Cannot get data.\n");
		    }
		}

		// IS_SERVER_UP
		{
			std::unique_ptr<CBuffer> buffer(p.SendAndGet(8, PACKET_SERVER::HEADER_CG_TEXT, "IS_SERVER_UP"));

			if (buffer) {
				std::string resText = buffer->Decode<std::string>();

				printf("IS_SERVER_UP Result: %s\n", resText.c_str());
			}
			else {
				printf("[IS_SERVER_UP]Cannot get data.\n");
			}
		}

		// NOTICE
		{
			std::unique_ptr<CBuffer> buffer(p.SendAndGet(16, PACKET_SERVER::HEADER_CG_TEXT, "NOTICE This is simple notice."));

			if (buffer) {
				std::string resText = buffer->Decode<std::string>();
			}
			else {

			}
		}

		// HEADER_CG_STATE_CHECKER
		{
			std::unique_ptr<CBuffer> buffer(p.SendAndGet(128, PACKET_SERVER::HEADER_CG_STATE_CHECKER));

			if (buffer) {
				BYTE bHeader = buffer->Decode<BYTE>();
				int nSize = buffer->Decode<int>();

				std::vector<TChannelStatus> collect;
				collect.reserve(nSize);

				for (int i = 0; i < nSize; i++)
					collect.emplace_back(buffer->Decode<TChannelStatus>());

				BYTE bSuccess = buffer->Decode<BYTE>();

				printf("HEADER_CG_STATE_CHECKER Result(%d): \n", nSize);
				for (const auto& v : collect)
					printf("\tPORT: %d -> STATUS: %d\n", v.nPort, v.bStatus);
			}
			else {
				printf("[HEADER_CG_STATE_CHECKER]Cannot get data.\n");
			}
		}
	}
	catch (std::exception& ex)
	{
		printf("\nError: %s\n", ex.what());
	}
	catch (...)
	{
		printf("\nError: Unhandled Exception.\n");
	}

	printf("\nFinished, Press a key.\n");
	std::cin.get();

	return EXIT_SUCCESS;
}
