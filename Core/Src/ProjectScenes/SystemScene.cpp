#include "ProjectScenes/SystemScene.hpp"
#include "ProjectScenes/CreateCharInfo.hpp"
#include <Input/SystemInputID.hpp>

#include <TaskHandler/PriorityTaskSheduler.hpp>
#include <Time/TimeUtils.hpp>
#include <DrawFunctions/DrawText.hpp>

#include <Graphics/DefaultFont.hpp>


#ifndef __STM32__
uint32_t __DummyGetter() { return 123456; }
#endif

class CPUScene : public IScene
{
protected:
	void DrawValue(IScreen& screen, Point2_i& pos, const char* name, uint32_t (*Getter)())
	{
		using namespace DrawFunctions;
		StaticText32 str;
		snprintf(str.CharPtr(), str.Capacity(), "%s: %d", name, Getter());
		DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
		pos.y += 8;
	}

public:
	CPUScene(ISceneManager* scene_manager) : IScene{scene_manager} {}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;
		
		Point2_i pos{0, 0};
		DrawText(screen, pos, "========CPU========");
		pos.y += 8;

#ifdef __STM32__
		DrawValue(screen, pos, "SysClock", &HAL_RCC_GetSysClockFreq);
		DrawValue(screen, pos, "HCLK",     &HAL_RCC_GetHCLKFreq);
		DrawValue(screen, pos, "PCLK1",    &HAL_RCC_GetPCLK1Freq);
		DrawValue(screen, pos, "PCLK2",    &HAL_RCC_GetPCLK2Freq);
#else
		DrawValue(screen, pos, "SysClock", &__DummyGetter);
		DrawValue(screen, pos, "HCLK",     &__DummyGetter);
		DrawValue(screen, pos, "PCLK1",    &__DummyGetter);
		DrawValue(screen, pos, "PCLK2",    &__DummyGetter);
#endif
	}

	bool ProcessInput(const AppEvent& event) override
	{
		if (IsSystemPrevEvent(event) || IsSystemNextEvent(event))
		{
			return true;
		}
		return false;
	}
	void Loop() override { }
};

// Страница с информацией о текущих задачах
class TasksScene : public IScene
{
protected:
	int start_from = 0;

public:
	TasksScene(ISceneManager* scene_manager) : IScene{scene_manager} {}

	void Draw(IScreen& screen) override
	{
		int i = 0, skip = start_from;
		for (const PriorityTask* task : PriorityTaskSheduler::GetList())
		{
			if (skip > 0) { skip--; continue; }
			DrawFunctions::DrawText(screen, {0, (i++) * 8}, CreateCharInfo(task->GetName(), task->GetAverageDurationMs(), 2));
			if (i >= 8) break;
		}
	}
	bool ProcessInput(const AppEvent& event) override
	{
		if (IsSystemPrevEvent(event))
		{
			if (start_from > 0) start_from--;
			return true;
		}
		else if (IsSystemNextEvent(event))
		{
			if (start_from < (int)PriorityTaskSheduler::GetList().Count() - 8) start_from++;
			return true;
		}
		return false;
	}
	void Loop() override { }
};

#include <Utils/random.hpp>
#include <Time/DateTime.hpp>
#include <Time/CurrentTimeCell.hpp>
#include <UI/Menu/Menu.hpp>
#include <UI/DateTimeSettingUI.hpp>
#include <UI/ValueSettingUI.hpp>
#include <UI/ListSettingUI.hpp>
#include <UI/SettingsContainer.hpp>
#include <UI/ButtonSettingUI.hpp>


class TimeScene : public IScene
{
protected:
	SettingsContainer settings;
	CurrentTimeCell cell_ms, cell_mks;

	ISettingUI *time, *date;
	ISettingUI *edit_time, *edit_date;
	ISettingUI *button;

	int32_t ppm{}, appm{};

	void OnChange()
	{
		time->SetActive(false);
		date->SetActive(false);
		edit_time->SetActive(true);
		edit_date->SetActive(true);
		button->SetActive(false);
	}

	void OnResetPPM()
	{
		DateTime::ResetActivePPM();
	}

public:
	TimeScene(ISceneManager* scene_manager)
		: IScene{scene_manager},
		settings { "===== ВРЕМЯ =====", &scene_manager->GetCommonAllocator() }, cell_ms{ false }, cell_mks{ true }
	{
		time = settings.AddSetting<DateTimeSettingUI>("ВРЕМЯ", Point2_i{-1, -1}, DateTimeSettingUI::TIME);
		date = settings.AddSetting<DateTimeSettingUI>("ДАТА", Point2_i{-1, -1}, DateTimeSettingUI::DATE);

		edit_time = settings.AddSetting<DateTimeEditableSettingUI>(">ВРЕМЯ", Point2_i{-1, -1}, DateTimeSettingUI::TIME);
		edit_date = settings.AddSetting<DateTimeEditableSettingUI>(">ДАТА", Point2_i{-1, -1}, DateTimeSettingUI::DATE);
		edit_time->SetActive(false);
		edit_date->SetActive(false);

		settings.AddSetting<ValueSettingUI<int32_t>>("PPM", Point2_i{-1, -1}, &ppm, "%" PRIi32);
		settings.AddSetting<ValueSettingUI<int32_t>>("APPM", Point2_i{-1, -1}, &appm, "%" PRIi32);
		settings.AddSetting<ValueSettingUI<uint32_t>>("MS", Point2_i{-1, -1}, &cell_ms, "%" PRIu32);
		settings.AddSetting<ValueSettingUI<uint32_t>>("MKS", Point2_i{-1, -1}, &cell_mks, "%" PRIu32);
		button = settings.AddSetting<ButtonCallInstanceSettingUI<TimeScene>>("ИЗМЕНИТЬ", Point2_i{-1, -1}, this, &TimeScene::OnChange);
		settings.AddSetting<ButtonCallInstanceSettingUI<TimeScene>>("СБРОС PPM", Point2_i{-1, -1}, this, &TimeScene::OnResetPPM);
		settings.Enable();

		AddObject(&settings);
	}

	void Draw(IScreen& screen) override {  }

	bool ProcessInput(const AppEvent& event) override
	{
		return IsSystemPrevEvent(event) || IsSystemNextEvent(event);
	}
	
	void Loop() override 
	{ 
		ppm = RTC_Settings::PPM.GetOrDefault().GetPPM();
		appm = DateTime::GetActivePPM();
	}
};

class RandomScene : public IScene
{
protected:
	float R{};
	int R_int{};

public:
	RandomScene(ISceneManager* scene_manager) : IScene{scene_manager} {}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		StaticText32 debug_str{};

		Point2_i p;

		snprintf(debug_str.CharPtr(), debug_str.Capacity(), "RAND: %0.6f", R);
		DrawText(screen, p, debug_str);
		p.y += 8;

		snprintf(debug_str.CharPtr(), debug_str.Capacity(), "RAND_INT: %d", R_int);
		DrawText(screen, p, debug_str);
	}

	bool ProcessInput(const AppEvent& event) override
	{
		if (IsSystemEnterEvent(event))
		{
			R = RandomFloat();
			R_int = Random(0, 9);
			return true;
		}
		else if (IsSystemPrevEvent(event) || IsSystemNextEvent(event))
		{
			return true;
		}
		return false;
	}
	void Loop() override { }
};


#include <ExtDevice/UI/UI_ExtDeviceStatus.hpp>
#include <SDCard/SDCard_ExtDevice.hpp>

class DevicesScene : public IScene
{
	StaticList<UI_ExtDeviceStatus, 16> ui_devs;
public:
	DevicesScene(ISceneManager* scene_manager) : IScene{scene_manager}
	{
		for (ExtDevice* device : ExtDevices)
		{
			UI_ExtDeviceStatus status;
			status.SetFont(&Default_Font_7x7_small);
			status.SetDevice(device);
			ui_devs.push_back(status);
		}
	}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		uint8_t idx = 0;
		for (UI_ExtDeviceStatus& dev : ui_devs)
		{
			dev.Draw(screen, {0, idx * 16});
			idx++;
		}
	}
	bool ProcessInput(const AppEvent& event) override 
	{
		bool proccesed = false;
		for (UI_ExtDeviceStatus& dev : ui_devs)
		{
			proccesed = dev.ProcessInput(event) || proccesed;
		}
		return proccesed;
	}
	void Loop() override { }
};

#include <LetoAPI_V1_System/Web/WebDevicesList.hpp>

class NearScene : public IScene
{
public:
	NearScene(ISceneManager* scene_manager) : IScene{scene_manager} {}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		uint8_t idx = 0;
		for (WebDeviceInfo_ListItem& info : WebDevicesList)
		{
			StaticText32 str{};
			snprintf(str.CharPtr(), str.Capacity(), "ch[%d] %s", info.device.channel, info.device.web_name);
			DrawText(screen, {0, idx * 16}, str);
			snprintf(str.CharPtr(), str.Capacity(), "%" PRIu32, info.last_activity);
			DrawText(screen, {0, idx * 16 + 8}, str);
			if (info.device.app_id)
			{
				snprintf(str.CharPtr(), str.Capacity(), "APP %X", info.device.app_id);
				DrawText(screen, {0, idx * 16 + 16}, str);
			}

			idx++;
			if (idx >= 3)
				break;
		}
		idx++;
	}
	bool ProcessInput(const AppEvent& event) override 
	{
		return false; 
	}
	void Loop() override { }
};

#include <LetoAPI_V1_System/Web/WebManager.hpp>
#include <LetoAPI_V1_System/Lobby/LobbyManager.hpp>
#include <System/DeviceID.hpp>

#include <Data/StaticList.hpp>

#include <GamesSupport/Lobby/LobbyData.hpp>
#include <GamesSupport/Lobby/LobbyDataProcessor.hpp>
#include <DrawFunctions/DrawRectangle.hpp>

#include <cstring>

struct AppEvent_LobbyData : public LobbyData
{
public:
	AppEvent_LobbyData(uint8_t id, uint32_t period) : LobbyData{ id, period } { }

	void SetData(AppEvent event) { send_event = event; }
	void GetData(AppEvent& event) { event = get_data; }

protected:
	AppEvent send_event;
	AppEvent get_data;

	bool OnSend(void* data, uint32_t size) override
	{
		if (size < sizeof(AppEvent)) return false;
		memcpy(data, &send_event, sizeof(AppEvent));
		return true;
	}

	void OnReceive(uint32_t /*id*/, const void* data, uint32_t size) override
	{
		if (size < sizeof(AppEvent)) return;
		memcpy(&get_data, data, sizeof(AppEvent));
	}
};

static LobbyDataProcessor* data_processor{};
static AppEvent_LobbyData app_event_data(0x01, 100);

void LobbyCallback(uint32_t id, const void* data, uint32_t size)
{
	if (data_processor)
		data_processor->ReceiveData(id, data, size);
}

class LobbySettingUI : public ISettingUI
{
protected:
	LobbyDataProcessor processor;

	StaticText32 CurrentValueRepr() override
	{
		return "";
	}

	void UpdateCurrentValue()
	{
	}

public:
	LobbySettingUI(StaticText32 name, Point2_i position)
		: ISettingUI{ name, position }
	{
		delimiter = false;

		processor.RegLobbyData(&app_event_data);
		data_processor = &processor;
	}
	~LobbySettingUI()
	{
		data_processor = nullptr;
	}

	void Draw(IScreen& screen, Point2_i offset = {}) override
	{
		LobbyConnection_V1 lobby;
		if (!LobbyManager_V1::Instance().GetActiveLobby(&lobby))
			return;

		snprintf(name.CharPtr(), name.Capacity(), "[%s] Lobby: %d", lobby.owner == GetDeviceID() ? "O" : "M", lobby.connected);

		ISettingUI::Draw(screen, offset);
		if (!IsCaptured())
			return;

		Point2_i start {30, 60};
		DrawFunctions::DrawOutlinedRectangle(screen, start, start + Point2_i{100, 60}, CyanColor, WhiteColor);

		StaticText32 str{};
		uint8_t i = 0;

		snprintf(str.CharPtr(), str.Capacity(), "O: %d", lobby.owner);
		DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		i++;

		AppEvent get_event;
		app_event_data.GetData(get_event);

		if (IsSystemNextEvent(get_event))
		{
			snprintf(str.CharPtr(), str.Capacity(), "Next");
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
		else if (IsSystemPrevEvent(get_event))
		{
			snprintf(str.CharPtr(), str.Capacity(), "Prev");
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}

		i++;

		for (uint32_t j = 0; j < lobby.connected; ++j, ++i)
		{
			snprintf(str.CharPtr(), str.Capacity(), "M[%d]: %d", j, lobby.members[j]);
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
	}

	bool ProcessInput(const AppEvent& event) override
	{
		// Если не выделен и не захвачен, обработка передается следующему элементу в списке
		// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
		if (!IsSelected())
			return false;

		if (!IsCaptured())
		{
			if (IsSystemEnterEvent(event, true))
			{
				Capture();
				return true;
			}
			return false;
		}

		if (IsSystemLeftEvent(event, true))
			Capture(false);

		app_event_data.SetData(event);

		return true;
	}

	void Loop() override { processor.Loop(); }
};

class LobbyNearSettingUI : public ISettingUI
{
protected:
	LobbyInfo_V1 lobbies_info[8];
	uint8_t lobbies_cnt;

	StaticText32 CurrentValueRepr() override
	{
		return "";
	}

	void UpdateCurrentValue()
	{
	}

public:
	LobbyNearSettingUI(StaticText32 name, Point2_i position)
		: ISettingUI{ name, position }
	{
		delimiter = false;
	}

	void Draw(IScreen& screen, Point2_i offset = {}) override
	{
		if (!IsCaptured())
		{
			ISettingUI::Draw(screen, offset);
			return;
		}

		StaticText32 str{};

		lobbies_cnt = LobbyManager_V1::Instance().GetLobbiesNear(lobbies_info, 8);

		Point2_i start {30, 30};
		DrawFunctions::DrawOutlinedRectangle(screen, start, {130, 90}, CyanColor, WhiteColor);
		for (uint8_t i = 0; i < lobbies_cnt; ++i)
		{
			snprintf(str.CharPtr(), str.Capacity(), "-Lobby %d", lobbies_info[i].owner);
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
		
		// Отрисовка доступных лобби
	}

	bool ProcessInput(const AppEvent& event) override
	{
		// Если не выделен и не захвачен, обработка передается следующему элементу в списке
		// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
		if (!IsSelected())
			return false;

		if (!IsCaptured())
		{
			if (IsSystemEnterEvent(event, true))
			{
				Capture();
				return true;
			}
			return false;
		}

		if (IsSystemEnterEvent(event))
		{
			LobbyManager_V1::Instance().JoinLobby(lobbies_info[0], &LobbyCallback);
		}

		if (IsSystemLeftEvent(event, true))
			Capture(false);

		return true;
	}
};

void CreateLobbyBtnCallback()
{
	LobbyManager_V1::Instance().CreateLobby(nullptr, 0x1, 3, &LobbyCallback);
}

void QuitLobbyBtnCallback()
{
	LobbyManager_V1::Instance().QuitLobby();
}

class WebScene : public IScene
{
protected:
	SettingsContainer settings;
	const uint16_t H = 8;

	ISettingUI* lobby{};
	ISettingUI* create{};
	ISettingUI* quit{};

public:
	WebScene(ISceneManager* scene_manager) 
		: IScene{scene_manager}, settings { "===== СВЯЗЬ =====", &scene_manager->GetCommonAllocator() }
	{
		create = settings.AddSetting<ButtonSettingUI>("СОЗДАТЬ", Point2_i{0, 4 * H}, &CreateLobbyBtnCallback);
		quit = settings.AddSetting<ButtonSettingUI>("ВЫЙТИ", Point2_i{0, 4 * H}, &QuitLobbyBtnCallback);
		quit->SetActive(false);

		settings.AddSetting<LobbyNearSettingUI>("РЯДОМ", Point2_i{0, 5 * H});

		lobby = settings.AddSetting<LobbySettingUI>("ЛОББИ", Point2_i{0, 6 * H});
		lobby->SetActive(false);

		settings.Enable();
		AddObject(&settings);
	}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		StaticText32 str{};
		snprintf(str.CharPtr(), str.Capacity(), "Buffer: %d", WebManager_V1::Instance().BufferPPS());
		DrawText(screen, {0, 1 * H}, str);

		snprintf(str.CharPtr(), str.Capacity(), "Connections: %d", WebManager_V1::Instance().GetConnections(nullptr, 0));
		DrawText(screen, {0, 2 * H}, str);

		LobbyConnection_V1 connection;
		if (LobbyManager_V1::Instance().GetActiveLobby(&connection))
		{
			lobby->SetActive(true);
			//snprintf(str.CharPtr(), str.Capacity(), "[%s] Lobby: %d", connection.owner == GetDeviceID() ? "O" : "M", connection.connected);
			//DrawText(screen, {0, 3 * H}, str);
			create->SetActive(false);
			quit->SetActive(true);
		}
		else
		{
			lobby->SetActive(false);
			create->SetActive(true);
			quit->SetActive(false);
		}
	}
	bool ProcessInput(const AppEvent& event) override { return false; }
	void Loop() override { }
};

#include <NRF24L01/NRF24L01_data.hpp>
#include <ProjectSettings/ProjectSettings.hpp>

namespace NRF24L01_UI
{
	static StaticList<ListSettingItem<uint8_t>, 6> pipes
	{
		{ "1", 1 },
		{ "2", 2 },
		{ "3", 3 },
		{ "4", 4 },
		{ "5", 5 },
		{ "6", 6 },
	};

	class ScanSettingUI : public ISettingUI
	{
	protected:
		StaticText32 CurrentValueRepr() override
		{
			return "";
		}

		void UpdateCurrentValue()
		{
		}

	public:
		ScanSettingUI(StaticText32 name, Point2_i position)
			: ISettingUI{ name, position }
		{
			delimiter = false;
		}

		void Draw(IScreen& screen, Point2_i offset = {}) override
		{
			if (!IsCaptured())
				ISettingUI::Draw(screen, offset);
			else
			{
				NRF24L01::Scanner.Draw(screen, offset);
			}
		}

		bool ProcessInput(const AppEvent& event) override
		{
			// Если не выделен и не захвачен, обработка передается следующему элементу в списке
			// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
			if (!IsSelected())
				return false;

			if (!IsCaptured())
			{
				if (IsSystemEnterEvent(event, true))
				{
					Capture();
					NRF24L01::Scanner.SetActive(true);
					return true;
				}
				return false;
			}

			if (IsSystemLeftEvent(event, true))
				Capture(false);

			return true;
		}
	};
};

#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

void basic_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	using namespace DrawFunctions;

	StaticText32 str{};
	snprintf(str.CharPtr(), str.Capacity(), "BIN: %c%c%c%c%c%c%c%c", BYTE_TO_BINARY(value));
	DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
}

struct RegBitInfo
{
	uint8_t bit;
	const char* name;
};

template <uint32_t N>
void reg_bit_draw(IScreen& screen, Point2_i& pos, uint8_t value, const RegBitInfo (&info)[N])
{
	using namespace DrawFunctions;
	for (const RegBitInfo& bit : info)
	{
		if (bit.bit != 0xFF)
		{
			StaticText32 str{};
			snprintf(str.CharPtr(), str.Capacity(), 
					"%d %s", (value >> bit.bit) & 0x1, bit.name);
			DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
		}
		pos.y += 7;
	}
}

#define REG_RESERVED "{ #9e9e9e }RESERVED{ # }"

template <typename T>
void draw_reg_cell_value(IScreen& screen, Point2_i& pos, const char* name, IDataCell<T>& cell, bool hex = true)
{
	StaticText32 str{};
	snprintf(str.CharPtr(), str.Capacity(), hex ? "%s: %X" : "%s: %d", name, cell.GetOrDefault());
	DrawFunctions::DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
	pos.y += 7;
}

void config_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED  },
		{ 6, "MASK_RX_DR" },
		{ 5, "MASK_TX_DS" },
		{ 4, "MASK_MAX_RT" },
		{ 3, "EN_CRC" },
		{ 2, "CRC0" },
		{ 1, "PWR_UP" },
		{ 0, "PRIM_RX" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void en_aa_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED  },
		{ 6, REG_RESERVED  },
		{ 5, "ENAA_P5" },
		{ 4, "ENAA_P4" },
		{ 3, "ENAA_P3" },
		{ 2, "ENAA_P2" },
		{ 1, "ENAA_P1" },
		{ 0, "ENAA_P0" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void en_rxaddr_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED  },
		{ 6, REG_RESERVED  },
		{ 5, "ERX_P5" },
		{ 4, "ERX_P4" },
		{ 3, "ERX_P3" },
		{ 2, "ERX_P2" },
		{ 1, "ERX_P1" },
		{ 0, "ERX_P0" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void setup_aw_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED  },
		{ 6, REG_RESERVED  },
		{ 5, REG_RESERVED  },
		{ 4, REG_RESERVED  },
		{ 3, REG_RESERVED  },
		{ 2, REG_RESERVED  },
		{ 1, "AW:1" },
		{ 0, "AW:0" },
	};
	static const char* aw_map[]
	{
		"illegal",
		"3 bytes",
		"4 bytes",
		"5 bytes"
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
	pos.y -= 10;
	pos.x += 48;
	DrawFunctions::DrawText(screen, pos, aw_map[value & 0x3], GrayColor, BlackColor, true,
			&Default_Font_7x7_small);
}

void setup_retr_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, "ARD:3" },
		{ 6, "ARD:2" },
		{ 5, "ARD:1" },
		{ 4, "ARD:0" },
		{ 0xFF, "" },
		{ 3, "ARC:3" },
		{ 2, "ARC:2" },
		{ 1, "ARC:1" },
		{ 0, "ARC:0" },
	};
	static const char* ard_map[]
	{
		"250us",
		"500us",
		"750us",
		"1000us",
		
		"1250us",
		"1500us",
		"1750us",
		"2000us",
		
		"2250us",
		"2500us",
		"2750us",
		"3000us",

		"3250us",
		"3500us",
		"3750us",
		"4000us"
	};
	static const char* arc_map[]
	{
		"DISBLD",
		"1 RETR",
		"2 RETR",
		"3 RETR",
		"4 RETR",
		"5 RETR",
		"6 RETR",
		"7 RETR",
		"8 RETR",
		"9 RETR",
		"10RETR",
		"11RETR",
		"12RETR",
		"13RETR",
		"14RETR",
		"15RETR"
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	Point2_i pos_add = pos;
	pos_add.x += 55;
	pos_add.y += 3;
	reg_bit_draw(screen, pos, value, info);
	DrawFunctions::DrawText(screen, pos_add, ard_map[(value >> 4) & 0xF], GrayColor, BlackColor, true,
			&Default_Font_7x7_small);
	pos_add.y += 35;
	DrawFunctions::DrawText(screen, pos_add, arc_map[value & 0xF], GrayColor, BlackColor, true,
			&Default_Font_7x7_small);
}

void rf_ch_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	basic_reg_draw(screen, pos, value);
	pos.y += 14;

	StaticText32 str{};
	snprintf(str.CharPtr(), str.Capacity(), "Channel: %d", value);
	DrawFunctions::DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
}

void rf_setup_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, REG_RESERVED },
		{ 5, REG_RESERVED },
		{ 4, "PLL_LOCK" },
		{ 3, "RF_DR" },
		{ 2, "RF_PWR" },
		{ 1, "RF_PWR" },
		{ 0, "LNA_HCURR" },
	};
	static const char* rf_pwr_map[]
	{
		"-18dBm",
		"-12dBm",
		"-6dBm",
		"0dBm"
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
	pos.y -= 17;
	pos.x += 60;
	DrawFunctions::DrawText(screen, pos, rf_pwr_map[(value >> 1) & 0x3], GrayColor, BlackColor, true,
			&Default_Font_7x7_small);
}

void status_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, "RX_DR" },
		{ 5, "TX_DS" },
		{ 4, "MAX_RT" },
		{ 3, "RX_P_NO" },
		{ 2, "RX_P_NO" },
		{ 1, "RX_P_NO" },
		{ 0, "TX_FULL" },
	};
	static const char* rx_p_no_map[]
	{
		"P0",
		"P1",
		"P2",
		"P3",
		"P4",
		"P5",
		"-",
		"EMPTY"
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
	pos.y -= 24;
	pos.x += 65;
	DrawFunctions::DrawText(screen, pos, rx_p_no_map[(value >> 1) & 0x7], GrayColor, BlackColor, true,
			&Default_Font_7x7_small);
}

void observe_tx_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	
	StaticText32 str{};
	snprintf(str.CharPtr(), str.Capacity(), "PLOS_CNT: %d", (value >> 4) & 0xF);
	DrawFunctions::DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
	pos.y += 7;

	snprintf(str.CharPtr(), str.Capacity(), "ARC_CNT: %d", value & 0xF);
	DrawFunctions::DrawText(screen, pos, str, WhiteColor, BlackColor, false, &Default_Font_7x7_small);
}

void cd_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, REG_RESERVED },
		{ 5, REG_RESERVED },
		{ 4, REG_RESERVED },
		{ 3, REG_RESERVED },
		{ 2, REG_RESERVED },
		{ 1, REG_RESERVED },
		{ 0, "CD" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void fifo_status_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, "TX_REUSE" },
		{ 5, "TX_FULL" },
		{ 4, "TX_EMPTY" },
		{ 3, REG_RESERVED },
		{ 2, REG_RESERVED },
		{ 1, "RX_FULL" },
		{ 0, "RX_EMPTY" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void dynpd_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, REG_RESERVED },
		{ 5, "DPL_P5" },
		{ 4, "DPL_P4" },
		{ 3, "DPL_P3" },
		{ 2, "DPL_P2" },
		{ 1, "DPL_P1" },
		{ 0, "DPL_P0" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void feature_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	static const RegBitInfo info[]
	{
		{ 7, REG_RESERVED },
		{ 6, REG_RESERVED },
		{ 5, REG_RESERVED },
		{ 4, REG_RESERVED },
		{ 3, REG_RESERVED },
		{ 2, "EN_DPL" },
		{ 1, "EN_ACK_PAY" },
		{ 0, "EN_DYN_ACK" },
	};

	basic_reg_draw(screen, pos, value);
	pos.y += 14;
	reg_bit_draw(screen, pos, value, info);
}

void rx_addr_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	basic_reg_draw(screen, pos, value);
	pos.y += 14;

	DrawFunctions::DrawText(screen, pos, "    0X", WhiteColor, BlackColor, false, &Default_Font_7x7_small);
	pos.y += 7;
	
	draw_reg_cell_value(screen, pos, "P0", NRF24L01::Reg_0A);
	draw_reg_cell_value(screen, pos, "P1", NRF24L01::Reg_0B);
	draw_reg_cell_value(screen, pos, "P2", NRF24L01::Reg_0C);
	draw_reg_cell_value(screen, pos, "P3", NRF24L01::Reg_0D);
	draw_reg_cell_value(screen, pos, "P4", NRF24L01::Reg_0E);
	draw_reg_cell_value(screen, pos, "P5", NRF24L01::Reg_0F);
}

void tx_addr_reg_draw(IScreen& screen, Point2_i pos, uint8_t value)
{
	basic_reg_draw(screen, pos, value);
	pos.y += 14;

	DrawFunctions::DrawText(screen, pos, "    0X", WhiteColor, BlackColor, false, &Default_Font_7x7_small);
	pos.y += 7;
	
	draw_reg_cell_value(screen, pos, "TX", NRF24L01::Reg_10);
}

static DataCell<uint8_t> dummy;

static const struct
{
	StaticText8 reg;
	IDataCell<uint8_t>* cell;
	const char* name;
	void (*panel) (IScreen& screen, Point2_i pos, uint8_t value);
} 
NRF24L01_ReadItems[]
{
	{ "00", &NRF24L01::Reg_00, "CONFIG",      &config_reg_draw },
	{ "01", &NRF24L01::Reg_01, "EN_AA",       &en_aa_reg_draw },
	{ "02", &NRF24L01::Reg_02, "EN_RXADDR",   &en_rxaddr_reg_draw },
	{ "03", &NRF24L01::Reg_03, "SETUP_AW",    &setup_aw_reg_draw },
	{ "04", &NRF24L01::Reg_04, "SETUP_RETR",  &setup_retr_reg_draw },
	{ "05", &NRF24L01::Reg_05, "RF_CH",       &rf_ch_reg_draw },
	{ "06", &NRF24L01::Reg_06, "RF_SETUP",    &rf_setup_reg_draw },
	{ "07", &NRF24L01::Reg_07, "STATUS",      &status_reg_draw },
	{ "08", &NRF24L01::Reg_08, "OBSERVE_TX",  &observe_tx_reg_draw },
	{ "09", &NRF24L01::Reg_09, "CD",          &cd_reg_draw },
	{ "0X", &dummy,            "RX_ADDR",     &rx_addr_reg_draw },
	{ "10", &dummy,            "TX_ADDR",     &tx_addr_reg_draw },
	{ "17", &NRF24L01::Reg_17, "FIFO_STATUS", &fifo_status_reg_draw },
	{ "1C", &NRF24L01::Reg_1C, "DYNPD",       &dynpd_reg_draw },
	{ "1D", &NRF24L01::Reg_1D, "FEATURE",     &feature_reg_draw },
};

class NRF24L01ReadScene : public IScene
{
protected:
	SettingsContainer settings;
	
public:
	NRF24L01ReadScene(ISceneManager* scene_manager) 
		: IScene{scene_manager}, 
		settings { NRF24L01::PVariant.GetOrDefault() ? "====NRF24L01P===" : "====NRF24L01====", &scene_manager->GetCommonAllocator() }
	{
		dummy.Set(0xFF);
		for (auto item : NRF24L01_ReadItems)
		{
			settings.AddSetting<ValueSettingUI<uint8_t>>(item.reg, Point2_i{-1, -1}, item.cell, "%02X");
		}
		
		settings.Enable();
		AddObject(&settings);
	}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;
	
		int cur = settings.GetCurrentSettingIdx();
		auto item = NRF24L01_ReadItems[cur];
			
		DrawText(screen, {58, 12}, item.name, strlen(item.name), WhiteColor, BlackColor, true, &Default_Font_7x7_small);
		
		NRF24L01_ReadItems[cur].panel(
				screen, 
				{58, 19}, 
				item.cell ? item.cell->GetOrDefault() : 0xFF);	
	}

	void Loop() override
	{
		settings.UpdateCurrentValue();
	}
};


class NRF24L01SettingScene : public IScene
{
protected:
	SettingsContainer settings;

public:
	NRF24L01SettingScene(ISceneManager* scene_manager) 
		: IScene{scene_manager}, 
		settings { NRF24L01::PVariant.GetOrDefault() ? "====NRF24L01P===" : "====NRF24L01====", &scene_manager->GetCommonAllocator() }
	{
		using namespace NRF24L01_UI;

		StaticListView<ListSettingItem<uint8_t>> pipes_view = pipes;

		settings.AddSetting<ListEditableSettingUI<uint8_t>>("RX-PIPE", Point2_i{-1, -1}, &NRF24L01::RxPipe, pipes_view, false);
		settings.AddSetting<ListEditableSettingUI<uint8_t>>("TX-PIPE", Point2_i{-1, -1}, &NRF24L01::TxPipe, pipes_view, false);
		settings.AddSetting<ValueEditableSettingUI<uint8_t>>("CHANNEL", Point2_i{-1, -1}, &NRF24L01::Channel, "%d", 0, 127, 1, false);

		if (NRF24L01::PVariant.GetOrDefault())
			settings.AddSetting<ScanSettingUI>("СКАНИРОВАТЬ", Point2_i{-1, -1});
		
		settings.Enable();
		AddObject(&settings);
	}
};

//class NRF24L01Scene : public CommonMenuScene
//{
//public:
//	NRF24L01Scene() : CommonMenuScene{ "====NRF24L01P===", 4, (uint8_t) SceneID::SYSTEM }
//	{
//		menu.AppendMenuItem("Регистры", new NRF24L01ReadScene);
//		menu.AppendMenuItem("Настройки", new NRF24L01SettingScene);
//	}
//};

#include <UI/CapacitySettingUI.hpp>
#include <SceneManager/SystemSceneManager.hpp>
#include <TaskHandler/PriorityTaskSheduler.hpp>

class CapacitySettingScene : public IScene
{
protected:
	SettingsContainer settings;

public:
	CapacitySettingScene(ISceneManager* scene_manager) : IScene{scene_manager}, settings{ "ПАМЯТЬ", &scene_manager->GetCommonAllocator() }
	{
		settings.AddSetting<CapacitySettingUI>("Scenes", Point2_i{-1, -1}, &scene_manager->GetCommonAllocator(), CapacitySettingsStyle::STYLE_1);
		settings.AddSetting<CapacitySettingUI>("Scenes", Point2_i{-1, -1}, &scene_manager->GetCommonAllocator(), CapacitySettingsStyle::STYLE_3);
		settings.AddSetting<CapacitySettingUI>("Tasks", Point2_i{-1, -1}, &scene_manager->GetCommonAllocator(), CapacitySettingsStyle::STYLE_1);
		settings.AddSetting<CapacitySettingUI>("Tasks", Point2_i{-1, -1}, &scene_manager->GetCommonAllocator(), CapacitySettingsStyle::STYLE_3);

		settings.Enable();
		AddObject(&settings);
	}

	void Loop() override
	{
		settings.UpdateCurrentValue();
	}

};

// ----------------------------------------------------------------------------------------------------

SystemScene::SystemScene(ISceneManager* scene_manager) 
	: CommonMenuScene{ scene_manager, "-----СИСТЕМА----", 12 }
{
	menu.AppendMenuItem("Процессор",	scene_manager->GetCommonAllocator().Make<CPUScene>(scene_manager));
	menu.AppendMenuItem("Задачи",		scene_manager->GetCommonAllocator().Make<TasksScene>(scene_manager));
	menu.AppendMenuItem("Устройства",	scene_manager->GetCommonAllocator().Make<DevicesScene>(scene_manager));
	menu.AppendMenuItem("Рядом",		scene_manager->GetCommonAllocator().Make<NearScene>(scene_manager));
	menu.AppendMenuItem("Связь",		scene_manager->GetCommonAllocator().Make<WebScene>(scene_manager));
	menu.AppendMenuItem("Память",		scene_manager->GetCommonAllocator().Make<CapacitySettingScene>(scene_manager));
	menu.AppendMenuItem("NRF24L01-R",	scene_manager->GetCommonAllocator().Make<NRF24L01ReadScene>(scene_manager));
	menu.AppendMenuItem("NRF24L01-S",	scene_manager->GetCommonAllocator().Make<NRF24L01SettingScene>(scene_manager));
	menu.AppendMenuItem("Время",		scene_manager->GetCommonAllocator().Make<TimeScene>(scene_manager));
	menu.AppendMenuItem("Рандом",		scene_manager->GetCommonAllocator().Make<RandomScene>(scene_manager));
}

// ----------------------------------------------------------------------------------------------------

