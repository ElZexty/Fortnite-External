#include "structs.h"

HRESULT directx_init()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_object)))
		exit(3);
	ZeroMemory(&p_params, sizeof(p_params));
	p_params.Windowed = TRUE;
	p_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_params.hDeviceWindow = my_wnd;
	p_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_params.BackBufferWidth = width;
	p_params.BackBufferHeight = height;
	p_params.EnableAutoDepthStencil = TRUE;
	p_params.AutoDepthStencilFormat = D3DFMT_D16;
	p_params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	p_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if (FAILED(p_object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, my_wnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_params, 0, &p_device)))
	{
		p_object->Release();
		exit(4);
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(my_wnd);
	ImGui_ImplDX9_Init(p_device);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;

	io.Fonts->AddFontFromFileTTF("C:\\font.ttf", 16);

	//D3DXCreateFontA(pDevice, 20, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Tahoma", &m_font);

	p_object->Release();
	return S_OK;
}

void create_overlay()
{
	WNDCLASSEXA wcsex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		("UDUDUDUD"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	RegisterClassExA(&wcsex);
	my_wnd = CreateWindowExA(NULL, "UDUDUDUD", "UDUDUDUD", WS_POPUP, rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, wcsex.hInstance, NULL);
	SetWindowLong(my_wnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(my_wnd, &margin);
	ShowWindow(my_wnd, SW_SHOW);
	SetWindowPos(my_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(my_wnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	UpdateWindow(my_wnd);
}

void DrawLine(int x1, int y1, int x2, int y2, ImColor color, int thickness)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(color), thickness);
}

void cleanup_d3d()
{
	if (p_device != NULL)
	{
		p_device->EndScene();
		p_device->Release();
	}
	if (p_object != NULL)
	{
		p_object->Release();
	}
}

DWORD_PTR nigga;

void actor_loop()
{
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	float closestDistance = FLT_MAX;
	DWORD_PTR closestPawn = NULL;
	Uworld = KmDrv->Rpm<DWORD_PTR>(base_address + GWorld);
	DWORD_PTR Gameinstance = KmDrv->Rpm<DWORD_PTR>(Uworld + OFFSETS::Gameinstance);
	DWORD_PTR LocalPlayers = KmDrv->Rpm<DWORD_PTR>(Gameinstance + OFFSETS::LocalPlayers);
	Localplayer = KmDrv->Rpm<DWORD_PTR>(LocalPlayers);
	PlayerController = KmDrv->Rpm<DWORD_PTR>(Localplayer + OFFSETS::PlayerController);
	LocalPawn = KmDrv->Rpm<DWORD_PTR>(PlayerController + OFFSETS::LocalPawn);
	PlayerState = KmDrv->Rpm<DWORD_PTR>(LocalPawn + OFFSETS::PlayerState);
	Rootcomp = KmDrv->Rpm<DWORD_PTR>(LocalPawn + OFFSETS::RootComponet);
	DWORD_PTR GameState = KmDrv->Rpm<DWORD_PTR>(Uworld + OFFSETS::GameState);
	DWORD_PTR PlayerArray = KmDrv->Rpm<DWORD_PTR>(GameState + OFFSETS::PlayerArray);

	for (int i = 0; i < settings::esp_distance; i++)
	{
		auto player = KmDrv->Rpm<uintptr_t>(PlayerArray + i * 0x8);
		auto CurrentActor = KmDrv->Rpm<uintptr_t>(player + OFFSETS::CurrentActor);
		uint64_t CurrentActorMesh = KmDrv->Rpm<uint64_t>(CurrentActor + OFFSETS::Mesh);
		int MyTeamId = KmDrv->Rpm<int>(PlayerState + OFFSETS::TeamId);
		DWORD64 otherPlayerState = KmDrv->Rpm<uint64_t>(CurrentActor + 0x290);
		int ActorTeamId = KmDrv->Rpm<int>(otherPlayerState + OFFSETS::TeamId);

		Vector3 Headpos = GetBoneWithRotation(CurrentActorMesh, 68);
		localactorpos = KmDrv->Rpm<Vector3>(Rootcomp + 0x128);
		float distance = localactorpos.Distance(Headpos) / 80;
		Vector3 bone66 = GetBoneWithRotation(CurrentActorMesh, 66);
		Vector3 top = ProjectWorldToScreen(bone66);
		Vector3 bone0 = GetBoneWithRotation(CurrentActorMesh, 0);
		Vector3 bottom = ProjectWorldToScreen(bone0);
		Vector3 Headbox = ProjectWorldToScreen(Vector3(Headpos.x, Headpos.y, Headpos.z + 15));
		Vector3 w2shead = ProjectWorldToScreen(Headpos);
		Vector3 vHeadBone = GetBoneWithRotation(CurrentActorMesh, 68);
		Vector3 vRootBone = GetBoneWithRotation(CurrentActorMesh, 0);
		Vector3 vHeadBoneOut = ProjectWorldToScreen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
		Vector3 vRootBoneOut = ProjectWorldToScreen(vRootBone);

		if (CurrentActor == LocalPawn) continue;

		float BHeight = abs(Headbox.y - bottom.y);
		float BWidth = BHeight * 0.75f;

		if (distance < settings::esp_distance)
		{
			if (settings::esp)
			{
				if (settings::box)
				{
					Render::Rect(Headbox.x - (BWidth / 2), Headbox.y, BWidth, BHeight, ImColor(0, 0, 0), 3);
					Render::Rect(Headbox.x - (BWidth / 2), Headbox.y, BWidth, BHeight, ImColor(0, 255, 255), 1);
				}

				if (settings::dynamic_esp)
				{
					Vector3 bottom1 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom2 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom3 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom4 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 1, vRootBone.z));

					Vector3 top1 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y, vHeadBone.z + 15));
					Vector3 top2 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y, vHeadBone.z + 15));

					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ImColor(0, 0, 0, 255), 3.1f);

					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ImColor(0, 0, 0, 255), 3.1f);

					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ImColor(255, 255, 255, 255), 0.1f);
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ImColor(255, 255, 255, 255), 0.1f);

					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ImColor(255, 255, 255, 255), 0.1f);
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ImColor(255, 255, 255, 255), 0.1f);
				}

				if (settings::line_esp)
				{

				}
			}

			if (settings::distance)
			{
				char name[64];
				sprintf_s(name, xor ("[ %2.fm ]"), distance);
				Render::ShadowText(Headbox.x - 25, Headbox.y - 15, ImColor(255, 255, 255), name);
			}

			auto dx = w2shead.x - (width / 2);
			auto dy = w2shead.y - (height / 2);
			auto dist = sqrtf(dx * dx + dy * dy) / 50;

			if (dist < settings::fov_rad && dist < closestDistance) {
				closestDistance = dist;
				closestPawn = CurrentActor;
			}

			if (closestPawn != 0)
			{
				if (closestPawn && GetAsyncKeyState(VK_RBUTTON))
				{
					if (settings::aimbot)
					{
						Vector3 hitbone = ProjectWorldToScreen(GetBoneWithRotation(playerHitbox, nigga));

						if (hitbone.x != 0 || hitbone.y != 0 || hitbone.z != 0)
						{
							if (settings::aimbot && closestPawn && GetAsyncKeyState(VK_RBUTTON) < 0) {
								AimAt(closestPawn);
							}
						}
					}
				}
			}
			else
			{
				closestDistance = FLT_MAX;
				closestPawn = NULL;
			}
		}
	}
}

void render_menu()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		settings::menu = !settings::menu;
	}

	if (settings::menu)
	{

		ColorChange();
		ImVec4* colors = ImGui::GetStyle().Colors;

		ImGui::SetNextWindowSize({ 500, 300 });
		ImGui::Begin("", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

		int x = 30;
		int y = 20;

		p = ImGui::GetWindowPos();
		// Main
		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(2, 2);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImColor(255, 0, 255);
		style->Colors[ImGuiCol_BorderShadow] = ImColor(255, 255, 255);
		style->Colors[ImGuiCol_FrameBg] = ImVec4();
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4();
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4();
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0, 0, 0, 255);

		Render::RectFilled(p.x, p.y - 50, 500, 40, ImColor(35, 35, 35));
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(0, 0, 0), 17);

		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(100, 100, 100), 14);
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(75, 75, 75), 12);
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(0, 0, 0), 11);
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(45, 45, 45), 10);
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(15, 15, 15), 3);
		Render::Rect(p.x, p.y - 50, 500, 30, ImColor(255, 0, 128), 1);

		Render::Rect(p.x - 10, p.y - 10, 520, 320, ImColor(0, 0, 0), 4);
		Render::Rect(p.x - 5, p.y - 5, 510, 310, ImColor(55, 55, 55), 11);
		Render::Rect(p.x, p.y, 500, 300, ImColor(0, 0, 0), 17);

		Render::Rect(p.x, p.y, 500, 300, ImColor(100, 100, 100), 14);
		Render::Rect(p.x, p.y, 500, 300, ImColor(75, 75, 75), 12);
		Render::Rect(p.x, p.y, 500, 300, ImColor(0, 0, 0), 11);
		Render::Rect(p.x, p.y, 500, 300, ImColor(45, 45, 45), 10);
		Render::Rect(p.x, p.y, 500, 300, ImColor(15, 15, 15), 3);
		Render::Rect(p.x, p.y, 500, 300, ImColor(255, 0, 128), 1);

		Render::ShadowText(p.x + 200, p.y - 45, ImColor(255, 255, 255), xor ("GelatoPack#2797"));

		PS::Tab(xor("Aimbot"), 1, 25, 10);
		PS::Tab(xor("Visuals"), 2, 185, 10);
		PS::Tab(xor("More"), 3, 350, 10);

		PS::ExitButton(xor ("Panic Button"), 345, 50);

		if (tabs == 1)
		{
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(55, 55, 55), 4);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(0, 0, 0), 3);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(255, 0, 128), 1);

			PS::Checkbox(xor ("Aimbot"), &settings::aimbot, 30, 70, ImColor(255, 0, 128));
			PS::Checkbox(xor ("Aimbot FOV"), &settings::aimbot_fov, 30, 95, ImColor(255, 0, 128));
			PS::Checkbox(xor ("In Player Crosshair"), &settings::inplayercrosshair, 30, 120, ImColor(255, 0, 128));
			PS::SliderFloat(xor ("FOV Radius"), &settings::fov_rad, 25, 1000, 30, 175);
			PS::SliderFloat(xor ("Strength"), &settings::smoothing, 1, 10, 30, 230);
		}

		if (tabs == 2)
		{
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(55, 55, 55), 4);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(0, 0, 0), 3);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(255, 0, 128), 1);

			PS::Checkbox(xor("Player Box ESP"), &settings::box, 30, 70, ImColor(255, 0, 128));
			PS::Checkbox(xor("Distance ESP"), &settings::distance, 30, 95, ImColor(255, 0, 128));
			PS::Checkbox(xor("Filled ESP"), &settings::filled, 30, 120, ImColor(255, 0, 128));
			PS::Checkbox(xor("Dynamic Box ESP"), &settings::dynamic_esp, 30, 145, ImColor(255, 0, 128));
		}

		if (tabs == 3)
		{
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(55, 55, 55), 4);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(0, 0, 0), 3);
			Render::Rect(p.x + 23, p.y + 50, 450, 220, ImColor(255, 0, 128), 1);
		}

		ImGui::End();
	}
}

WPARAM render_loop()
{
	static RECT old_rc;
	ZeroMemory(&messager, sizeof(MSG));
	while (messager.message != WM_QUIT)
	{
		if (PeekMessage(&messager, my_wnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&messager);
			DispatchMessage(&messager);
		}
		HWND hwnd_active = GetForegroundWindow();
		if (GetAsyncKeyState(0x23) & 1)
			exit(8);
		if (hwnd_active == game_wnd)
		{
			HWND hwnd_test = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(my_wnd, hwnd_test, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;
		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(game_wnd, &rc);
		ClientToScreen(game_wnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = game_wnd;
		io.DeltaTime = 1.0f / 60.0f;
		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1))
		{
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
		{
			io.MouseDown[0] = false;
		}
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;
			width = rc.right;
			height = rc.bottom;
			p_params.BackBufferWidth = width;
			p_params.BackBufferHeight = height;
			SetWindowPos(my_wnd, (HWND)0, xy.x, xy.y, width, height, SWP_NOREDRAW);
			p_device->Reset(&p_params);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		render_menu();
		actor_loop();

		if (settings::aimbot_fov)
		{
			ImGui::GetForegroundDrawList()->AddCircle({ width / 2, height / 2 }, settings::fov_rad, ImColor(255, 0, 255), 55);
		}

		ImGui::EndFrame();
		p_device->SetRenderState(D3DRS_ZENABLE, false);
		p_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		p_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		p_device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		if (p_device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			p_device->EndScene();
		}
		HRESULT result = p_device->Present(NULL, NULL, NULL, NULL);
		if (result == D3DERR_DEVICELOST && p_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_device->Reset(&p_params);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	cleanup_d3d();
	DestroyWindow(my_wnd);
	return messager.wParam;
}