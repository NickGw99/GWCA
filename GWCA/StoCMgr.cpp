#include "StoCMgr.h"

#include "GWAPIMgr.h"
#include "PatternScanner.h"

GWAPI::StoCMgr::StoCHandlerArray GWAPI::StoCMgr::game_server_handler_;
GWAPI::StoCMgr::StoCHandler* GWAPI::StoCMgr::original_functions_ = NULL;
std::map<DWORD, std::vector<GWAPI::StoCMgr::Handler>> GWAPI::StoCMgr::event_calls_;

bool GWAPI::StoCMgr::StoCHandlerFunc(StoC::PacketBase* pak, DWORD unk)
{
	for (auto call : event_calls_[pak->header])
	{
		call(pak);
	}
	return original_functions_[pak->header].handlerfunc(pak, unk);
}

GWAPI::StoCMgr::StoCMgr(GWAPIMgr& api) : GWCAManager(api) {
	PatternScanner scan(0x401000, 0x49A000);
	
	// inb4 has rages at this
	struct LSObjPtrChain {
		struct {
			struct {
				BYTE pad[0x14];
				struct {
					BYTE pad[0x8];
					struct {
						BYTE pad[0x8];
						struct {
							BYTE pad[0x24];
							StoCHandlerArray gshandlers;
						} *sub4;
					} *sub3;
				} *sub2;
			};
		}*sub1;
	} *lsobjbase = *(LSObjPtrChain**)scan.FindPattern("\x8B\x56\x04\x85\xC0\x89\x57\x18", "xxxxxxxx", -4);

	game_server_handler_ = lsobjbase->sub1->sub2->sub3->sub4->gshandlers;

	original_functions_ = new StoCHandler[game_server_handler_.size()];

	for (DWORD i = 0; i < game_server_handler_.size(); ++i) {
		original_functions_[i] = game_server_handler_[i];
	}

}

void GWAPI::StoCMgr::RestoreHooks() {
	for (DWORD i = 0; i < game_server_handler_.size(); ++i) {
		game_server_handler_[i] = original_functions_[i];
	}

	delete[] original_functions_;
}
