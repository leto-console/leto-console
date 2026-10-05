#include "ProjectScenes/DebugScene.hpp"
#include <Input/SystemInputID.hpp>

#include "Common/ProjectGraphs.hpp"

#include <SSD1306/SSD1306_Properties.hpp>

#include <Utils/crc16.hpp>
#include "ProjectScenes/CreateCharInfo.hpp"

#include <DrawFunctions/DrawText.hpp>

#include <SceneManager/ISceneManager.hpp>

#include <LetoFunctions/Draw.hpp>
#include <Graphics/Fonts/base_6x6/base_6x6_font.hpp>

enum DEBUG_MODE
{
	_DM_ENCODERS,
	_DM_ALPHABET_ASCII,
	_DM_ALPHABET_RUS,
	_DM_BONUS,
	_DM_COUNT
};

static void DrawAlphabetPage(LetoScreen_V1* leto_screen, const char* (&lines)[3], RGBColor main, RGBColor back)
{
	Point2_i p{2, 4};

	leto_api_v1->Graphics->FillScreen(leto_screen, back);

	leto::graphics::DrawText(leto_screen, p, lines[0], nullptr, main, back); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[1], nullptr, main, back); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[2], nullptr, main, back); p.y += 10;

	leto::graphics::DrawText(leto_screen, p, lines[0], nullptr, main, back, {1, 1}); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[1], nullptr, main, back, {1, 1}); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[2], nullptr, main, back, {1, 1}); p.y += 10;
	
	leto::graphics::DrawText(leto_screen, p, lines[0], nullptr, back, main); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[1], nullptr, back, main); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[2], nullptr, back, main); p.y += 10;

	leto::graphics::DrawText(leto_screen, p, lines[0], nullptr, back, main, {1, 1}); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[1], nullptr, back, main, {1, 1}); p.y += 10;
	leto::graphics::DrawText(leto_screen, p, lines[2], nullptr, back, main, {1, 1}); p.y += 10;
}

void DebugScene::Draw(IScreen& screen)
{
	LetoScreen_V1* leto_screen = IScreen::ToHandle(&screen);

	switch (mode)
	{
	case _DM_ENCODERS:
	{
		static StaticText8 clk_text = "CLK";
		static StaticText8 dt_text = "DT";

		DrawFunctions::DrawText(screen, {0, 0}, clk_text);
		DEBUG_ENCODER_CLK.Draw(screen, {0, 0});
		DrawFunctions::DrawText(screen, {0, SSD1306_Height / 2}, dt_text);
		DEBUG_ENCODER_DT.Draw(screen, {0, SSD1306_Height / 2});

		//char text[256];
		//uint32_t data = 0xC1A0BABE;
		//snprintf(text, 256, "%04X", calc_crc16(&data, sizeof(data)));
		//
		//screen.SimpleText(50, 0, text);

		return;
	}
	case _DM_ALPHABET_ASCII:
	{
		const char* line1 = "AaBbCcDd{ #00ff00 }Ee{ # }FfGgHhIiJjKk{ #ff0000 }Ll{ # }";
		const char* line2 = "MmNn{ #0000ff }Oo{ # }PpQqRrSs{ #ff00ff }Tt{ # }Uu";
		const char* line3 = "VvWwXxYyZz";

		const char* lines[] { line1, line2, line3 };

		DrawAlphabetPage(leto_screen, lines, reverse ? BlackColor : WhiteColor, reverse ? WhiteColor : BlackColor);
		return;
	}
	case _DM_ALPHABET_RUS:
	{
		const char* line1 = "АаБбВвГгДд{ #00ff00 }Ее{ # }ЁёЖжЗзИиЙйКк";
		const char* line2 = "{ #ff0000 }Лл{ # }МмНн{ #0000ff }Оо{ # }ПпРрСс{ #ff00ff }Тт{ # }УуФфХхЦц";
		const char* line3 = "ЧчШшЩщЪъЫыЬьЭэЮюЯя";
		
		const char* lines[] { line1, line2, line3 };

		DrawAlphabetPage(leto_screen, lines, reverse ? BlackColor : WhiteColor, reverse ? WhiteColor : BlackColor);
		return;
	}
	case _DM_BONUS:
	{
		const char* poem_part1[]
		{
			"Грубым дается радость.",
			"Нежным дается печаль.",
			"Мне ничего не надо,",
			"Мне никого не жаль.",
			"",
			"Жаль мне себя немного,",
			"Жалко бездомных собак.",
			"Эта прямая дорога",
			"Меня привела в кабак.",
			"",
			"Что ж вы ругаетесь, дьяволы?",
			"Иль я не сын страны?",
			"Каждый из нас закладывал",
			"За рюмку свои штаны."
		};

		const char* poem_part2[]
		{
			"Мутно гляжу на окна.",
			"В сердце тоска и зной.",
			"Катится, в солнце измокнув,",
			"Улица передо мной.",
			"",
			"А на улице мальчик сопливый.",
			"Воздух поджарен и сух.",
			"Мальчик такой счастливый",
			"И ковыряет в носу.",
			"",
			"Ковыряй, ковыряй, мой милый,",
			"Суй туда палец весь,",
			"Только вот с эфтой силой",
			"В душу свою не лезь.",
		};

		const char* poem_part3[]
		{
			"",
			"",
			"",
			"",
			"",
			"Я уж готов.  Я робкий.",
			"Глянь на бутылок рать!",
			"Я собираю пробки -",
			"Душу мою затыкать.",
			"",
			"",
			"",
			"",
			"",
		};

		const char* (*poem)[14] = &poem_part1;

		if (bonus_part % 3 == 1) poem = &poem_part2;
		else if (bonus_part % 3 == 2) poem = &poem_part3;

		leto_api_v1->Graphics->FillScreen(leto_screen, reverse ? WhiteColor : BlackColor);

		int x = 8;
		for (const char*& line : *poem)
		{
			leto::graphics::DrawText(leto_screen, {8, x}, line, nullptr, reverse ? BlackColor : WhiteColor, reverse ? WhiteColor : BlackColor, {1,1});
			x += 8;
		}
		return;
	}

	default:
		break;
	}



}

bool DebugScene::ProcessInput(const AppEvent& event)
{
	if (IsSystemReturnEvent(event))
		scene_manager->Return();
	if (IsSystemAltEvent(event))
	{
		mode = (mode + 1) % _DM_COUNT;
		if (mode == _DM_BONUS) bonus_part = 0;
		reverse = false;
	}
	if (IsSystemEnterEvent(event))
	{
		reverse = !reverse;
	}
	if (IsSystemNextEvent(event) && mode == _DM_BONUS) bonus_part++;
	if (IsSystemPrevEvent(event) && mode == _DM_BONUS) bonus_part--;
	return true;
}
