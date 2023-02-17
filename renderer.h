#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <dwmapi.h>
#include <d3d9.h>
#include "driver.h"
#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")

#include <windows.h>
#include <winternl.h>
#include <process.h>
#include <tlhelp32.h>
#include <inttypes.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <d3d9.h>

#include "settings.h"
#include "lazyimporter.h"

ImVec2 p;

namespace Render
{
	struct Colors
	{
		ImColor red = { 255, 0, 0, 255 };
		ImColor green = { 0, 255, 0, 255 };
		ImColor blue = { 0, 136, 255, 255 };
		ImColor aqua_blue = { 0, 255, 255, 255 };
		ImColor cyan = { 0, 210, 210, 255 };
		ImColor royal_purple = { 102, 0, 255, 255 };
		ImColor dark_pink = { 255, 0, 174, 255 };
		ImColor black = { 0, 0, 0, 255 };
		ImColor white = { 255, 255, 255, 255 };
		ImColor purple = { 255, 0, 255, 255 };
		ImColor yellow = { 255, 255, 0, 255 };
		ImColor orange = { 255, 140, 0, 255 };
		ImColor gold = { 234, 255, 0, 255 };
		ImColor royal_blue = { 0, 30, 255, 255 };
		ImColor dark_red = { 150, 5, 5, 255 };
		ImColor dark_green = { 5, 150, 5, 255 };
		ImColor dark_blue = { 100, 100, 255, 255 };
		ImColor navy_blue = { 0, 73, 168, 255 };
		ImColor light_gray = { 200, 200, 200, 255 };
		ImColor dark_gray = { 150, 150, 150, 255 };
	};
	Colors color;

	void Text(int posx, int posy, ImColor clr, const char* text)
	{
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx, posy), ImColor(clr), text);
	}

	void OutlinedText(int posx, int posy, ImColor clr, const char* text)
	{
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx + 1, posy + 1), ImColor(color.black), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx - 1, posy - 1), ImColor(color.black), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx + 1, posy + 1), ImColor(color.black), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx - 1, posy - 1), ImColor(color.black), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx, posy), ImColor(clr), text);
	}

	void ShadowText(int posx, int posy, ImColor clr, const char* text)
	{
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx + 1, posy + 2), ImColor(0, 0, 0, 200), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx + 1, posy + 2), ImColor(0, 0, 0, 200), text);
		ImGui::GetForegroundDrawList()->AddText(ImVec2(posx, posy), ImColor(clr), text);
	}

	void Rect(int x, int y, int w, int h, ImColor color, int thickness)
	{
		ImGui::GetForegroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0, thickness);
	}

	void RectFilled(int x, int y, int w, int h, ImColor color)
	{
		ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0);
	}

	void RectFilledRound(int x, int y, int w, int h, ImColor color, float rounding)
	{
		ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), rounding, 0);
	}
}

int tabs = 1;

namespace PS
{
	void Tab(const char* v, int tab_next, float x, float y)
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Button] = ImVec4();
		colors[ImGuiCol_ButtonHovered] = ImVec4();
		colors[ImGuiCol_ButtonActive] = ImVec4();

		ImGui::SetCursorPos({ x, y });
		if (ImGui::Button(v, ImVec2(120, 35))) tabs = tab_next;

		ImVec2 pos = ImGui::GetWindowPos();

		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(25, 25, 25), 7);
		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(45, 45, 45), 5);
		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(0, 0, 0), 3);

		if (ImGui::IsItemHovered())
		{
			Render::RectFilled(pos.x + x, pos.y + y + 28, 120, 3, ImColor(150, 150, 150));
		}
		else
		{
			Render::RectFilled(pos.x + x, pos.y + y + 28, 120, 2, ImColor(255, 255, 255, 255));
		}
	}

	void ExitButton(const char* v, float x, float y)
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Button] = ImVec4();
		colors[ImGuiCol_ButtonHovered] = ImVec4();
		colors[ImGuiCol_ButtonActive] = ImVec4();

		ImGui::SetCursorPos({ x, y });
		if (ImGui::Button(v, ImVec2(120, 35)))
		{
			exit(1);
		}

		ImVec2 pos = ImGui::GetWindowPos();

		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(25, 25, 25), 7);
		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(45, 45, 45), 5);
		Render::Rect(pos.x + x, pos.y + y + 28, 120, 3, ImColor(0, 0, 0), 3);

		if (ImGui::IsItemHovered())
		{
			Render::RectFilled(pos.x + x, pos.y + y + 28, 120, 3, ImColor(150, 150, 150));
		}
		else
		{
			Render::RectFilled(pos.x + x, pos.y + y + 28, 120, 2, ImColor(255, 255, 255, 255));
		}
	}

	void SliderFloat(const char* v, float* setting, float min, float max, float x, float y)
	{
		ImGui::SetCursorPos({ x, y });
		ImGui::SliderFloat(v, setting, min, max);
	}

	void Checkbox(const char* v, bool* option, float x, float y, ImColor col)
	{
		ImGui::SetCursorPos({ x, y });
		ImGui::Checkbox(v, option);

		p = ImGui::GetWindowPos();

		Render::Rect(p.x + x + 2, p.y + y + 2, 14, 14, ImColor(0, 0, 0), 3);
		Render::Rect(p.x + x + 2, p.y + y + 2, 14, 14, ImColor(255, 255, 255), 1);
		Render::RectFilledRound(p.x + x + 2, p.y + y + 2, 14, 14, ImColor(20, 20, 20, 255), 0);

		if (*option)
		{
			Render::RectFilledRound(p.x + x + 2, p.y + y + 2, 14, 14, ImColor(col), 0);
		}
	}
}

float color_red = 1.;
float color_green = 0;
float color_blue = 0;
float color_random = 0.0;
float color_speed = -1;

void ColorChange()
{
	static float Color[3];
	static DWORD Tickcount = 0;
	static DWORD Tickcheck = 0;
	ImGui::ColorConvertRGBtoHSV(color_red, color_green, color_blue, Color[0], Color[1], Color[2]);
	if (GetTickCount() - Tickcount >= 1)
	{
		if (Tickcheck != Tickcount)
		{
			Color[0] += 0.001f * color_speed;
			Tickcheck = Tickcount;
		}
		Tickcount = GetTickCount();
	}
	if (Color[0] < 0.0f) Color[0] += 1.0f;
	ImGui::ColorConvertHSVtoRGB(Color[0], Color[1], Color[2], color_red, color_green, color_blue);
}