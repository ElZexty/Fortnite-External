#include "renderer.h"
#include "xorstr.h"

#define GWorld 0xEE3BA38

namespace OFFSETS
{
	// TeamIndex - TeamID - ActorID
	uintptr_t TeamId = 0x1100;

	// Local
	uintptr_t LocalActorPos = 0x128;
	uintptr_t LocalPawn = 0x330;
	uintptr_t LocalPlayers = 0x38;

	// Player - BoneArray
	uintptr_t BoneArray = 0x5E8;
	uintptr_t PlayerState = 0x2a8;
	uintptr_t PlayerArray = 0x2a0;
	uintptr_t PlayerController = 0x30;
	uintptr_t PlayerCameraManager = 0x340;

	// Game - World
	uintptr_t Gameinstance = 0x1B8;
	uintptr_t GameState = 0x158;
	uintptr_t ComponetToWorld = 0x240;

	// Actor
	uintptr_t ActorCount = 0xA0;
	uintptr_t AActor = 0x98;
	uintptr_t CurrentActor = 0x300;

	// Other
	uintptr_t RootComponet = 0x190;
	uintptr_t PersistentLevel = 0x30;
	uintptr_t Mesh = 0x310;
	uintptr_t Velocity = 0x170;
	uintptr_t PawnPrivate = 0x300;
	uintptr_t ReviveFromDBNOTime = 0x43c0;
}

DWORD_PTR LocalPawn;
DWORD_PTR PlayerState;
DWORD_PTR PlayerController;
DWORD_PTR Persistentlevel;
uintptr_t PlayerCameraManager;

class UClass {
public:
	BYTE _padding_0[0x40];
	UClass* SuperClass;
};

class UObject {
public:
	PVOID VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	UClass* Class;
	BYTE _padding_0[0x8];
	UObject* Outer;

	inline BOOLEAN IsA(PVOID parentClass) {
		for (auto super = this->Class; super; super = super->SuperClass) {
			if (super == parentClass) {
				return TRUE;
			}
		}

		return FALSE;
	}
};

class FUObjectItem {
public:
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
	DWORD SerialNumber2;
};

class TUObjectArray {
public:
	FUObjectItem* Objects[9];
};

class GObjects {
public:
	TUObjectArray* ObjectArray;
	BYTE _padding_0[0xC];
	DWORD ObjectCount;
};


DWORD FNProcID;
IDirect3D9Ex* p_object = NULL;
IDirect3DDevice9Ex* p_device = NULL;
D3DPRESENT_PARAMETERS p_params = { NULL };
HWND game_wnd = NULL;
RECT game_rect = { NULL };
HWND my_wnd = NULL;
MSG messager = { NULL };

float width, height;

Memory* KmDrv = nullptr;
uintptr_t base_address;

#define M_PI 3.14159265358979323846264338327950288419716939937510

class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	double GetDistance(double x1, double y1, double z1, double x2, double y2) {
		return sqrtf(powf((x2 - x1), 2) + powf((y2 - y1), 2));
	}


	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(float number) const {
		return Vector3(x * number, y * number, z * number);
	}
};

Vector3 localactorpos;

auto isVisible(uintptr_t test) -> bool
{
	float fLastSubmitTime = KmDrv->Rpm<float>(OFFSETS::Mesh + 0x358);
	float fLastRenderTimeOnScreen = KmDrv->Rpm<float>(OFFSETS::Mesh + 0x360);
	const float fVisionTick = 0.06f;
	bool bVisible = fLastRenderTimeOnScreen + fVisionTick >= fLastSubmitTime;
	return bVisible;
}

struct FQuat
{
	double x;
	double y;
	double z;
	double w;
};

struct FTransform
{
	FQuat rot;
	Vector3 translation;
	Vector3 scale;
	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

DWORD_PTR Uworld;
DWORD_PTR Rootcomp;
DWORD_PTR Localplayer;
#define PI 3.14159265358979323846f

struct _MATRIX {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;
		};
		float m[4][4];
	};
};

_MATRIX Matrix(Vector3 Vec4, Vector3 origin = Vector3(0, 0, 0))
{
	double radPitch = (Vec4.x * double(PI) / 180.f);
	double radYaw = (Vec4.y * double(PI) / 180.f);
	double radRoll = (Vec4.z * double(PI) / 180.f);
	double SP = sinf(radPitch);
	double CP = cosf(radPitch);
	double SY = sinf(radYaw);
	double CY = cosf(radYaw);
	double SR = sinf(radRoll);
	double CR = cosf(radRoll);
	_MATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;
	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;
	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;
	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;
	return matrix;
}

struct Camera
{
	float FieldOfView;
	Vector3 Rotation;
	Vector3 Location;
};

Camera GetCamera(__int64 a1)
{
	Camera LocalCamera;
	__int64 v1;
	v1 = KmDrv->Rpm<__int64>(Localplayer + 0xd0);
	__int64 v9 = KmDrv->Rpm<__int64>(v1 + 0x8); // 0x10
	LocalCamera.FieldOfView = 80.f / (KmDrv->Rpm<double>(v9 + 0x7F0) / 1.19f); // 0x600
	LocalCamera.Rotation.x = KmDrv->Rpm<double>(v9 + 0x9C0);
	LocalCamera.Rotation.y = KmDrv->Rpm<double>(a1 + 0x148);
	uint64_t FGC_Pointerloc = KmDrv->Rpm<uint64_t>(Uworld + 0x110);
	LocalCamera.Location = KmDrv->Rpm<Vector3>(FGC_Pointerloc);
	return LocalCamera;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
	Camera vCamera = GetCamera(Rootcomp);
	vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);
	_MATRIX tempMatrix = Matrix(vCamera.Rotation, Vector3(0, 0, 0));
	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);
	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
	if (vTransformed.z < 1.f) vTransformed.z = 1.f;
	return Vector3((width / 2.0f) + vTransformed.x * (((width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (height / 2.0f) - vTransformed.y * (((width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray;
	bonearray = KmDrv->Rpm<DWORD_PTR>(mesh + OFFSETS::BoneArray);

	if (bonearray == NULL)
	{
		bonearray = KmDrv->Rpm<DWORD_PTR>(mesh + OFFSETS::BoneArray + 0x10);
	}
	return  KmDrv->Rpm<FTransform>(bonearray + (index * 0x60));
}

Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = KmDrv->Rpm<FTransform>(mesh + OFFSETS::ComponetToWorld);

	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

float Smoothness = 5;

namespace MouseController
{
	static BYTE NtUserSendInput_Bytes[30];
	static BYTE NtUserGetAsyncKeyState_Bytes[30];

	// Call this once in DllMain or main
	static BOOLEAN WINAPI Init()
	{
		// windows 8.1 / windows 10
		LPVOID NtUserSendInput_Addr = GetProcAddress(GetModuleHandle("win32u"), "NtUserSendInput");
		if (!NtUserSendInput_Addr)
		{
			NtUserSendInput_Addr = GetProcAddress(GetModuleHandle("user32"), "NtUserSendInput");
			if (!NtUserSendInput_Addr)
			{
				// Windows 7 or lower detected
				NtUserSendInput_Addr = GetProcAddress(GetModuleHandle("user32"), "SendInput");
				if (!NtUserSendInput_Addr)
					return FALSE;
			}
		}

		// windows 8.1 / windows 10
		LPVOID NtUserGetAsyncKeyState_Addr = GetProcAddress(GetModuleHandle("win32u"), "NtUserGetAsyncKeyState");
		if (!NtUserGetAsyncKeyState_Addr)
		{
			NtUserGetAsyncKeyState_Addr = GetProcAddress(GetModuleHandle("user32"), "NtUserGetAsyncKeyState");
			if (!NtUserGetAsyncKeyState_Addr)
			{
				// Windows 7 or lower detected
				NtUserGetAsyncKeyState_Addr = GetProcAddress(GetModuleHandle("user32"), "GetAsyncKeyState");
				if (!NtUserGetAsyncKeyState_Addr)
					return FALSE;
			}
		}
		memcpy(NtUserSendInput_Bytes, NtUserSendInput_Addr, 30);
		memcpy(NtUserGetAsyncKeyState_Bytes, NtUserGetAsyncKeyState_Addr, 30);
		return TRUE;
	}

	/* This function spoofs the function. It prevents BattlEye from scanning it! */
	static BOOLEAN WINAPI NtUserSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
	{
		LPVOID NtUserSendInput_Spoof = VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE); // allocate space for syscall
		if (!NtUserSendInput_Spoof)
			return FALSE;
		memcpy(NtUserSendInput_Spoof, NtUserSendInput_Bytes, 30); // copy syscall
		NTSTATUS Result = reinterpret_cast<NTSTATUS(NTAPI*)(UINT, LPINPUT, int)>(NtUserSendInput_Spoof)(cInputs, pInputs, cbSize); // calling spoofed function
		ZeroMemory(NtUserSendInput_Spoof, 0x1000); // clean address
		VirtualFree(NtUserSendInput_Spoof, 0, MEM_RELEASE); // free it
		return (Result > 0); // return the status
	}

	/* This function spoofs the function. It prevents BattlEye from scanning it! */
	static UINT WINAPI NtUserGetAsyncKeyState(UINT Key)
	{
		LPVOID NtUserGetAsyncKeyState_Spoof = VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE); // allocate space for syscall
		if (!NtUserGetAsyncKeyState_Spoof)
			return FALSE;
		memcpy(NtUserGetAsyncKeyState_Spoof, NtUserGetAsyncKeyState_Bytes, 30); // copy syscall
		NTSTATUS Result = reinterpret_cast<NTSTATUS(NTAPI*)(UINT)>(NtUserGetAsyncKeyState_Spoof)(Key); // calling spoofed function
		ZeroMemory(NtUserGetAsyncKeyState_Spoof, 0x1000); // clean address
		VirtualFree(NtUserGetAsyncKeyState_Spoof, 0, MEM_RELEASE); // free it
		return Result; // return the status
	}

	/* This function moves the mouse using the syscall */
	static BOOLEAN WINAPI Move_Mouse(int X, int Y)
	{
		INPUT input;
		input.type = INPUT_MOUSE;
		input.mi.mouseData = 0;
		input.mi.time = 0;
		input.mi.dx = X;
		input.mi.dy = Y;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
		return NtUserSendInput(1, &input, sizeof(input));
	}

	/* This function gets key state using the syscall*/
	static UINT WINAPI GetAsyncKeyState(UINT Key)
	{
		return NtUserGetAsyncKeyState(Key);
	}
}

static auto move_to(float x, float y) -> void {
	float center_x = (ImGui::GetIO().DisplaySize.x / 2);
	float center_y = (ImGui::GetIO().DisplaySize.y / 2);

	float smooth = settings::smoothing;

	float target_x = 0.f;
	float target_y = 0.f;

	if (x != 0.f)
	{
		if (x > center_x)
		{
			target_x = -(center_x - x);
			target_x /= smooth;
			if (target_x + center_x > center_x * 2.f) target_x = 0.f;
		}

		if (x < center_x)
		{
			target_x = x - center_x;
			target_x /= smooth;
			if (target_x + center_x < 0.f) target_x = 0.f;
		}
	}

	if (y != 0.f)
	{
		if (y > center_y)
		{
			target_y = -(center_y - y);
			target_y /= smooth;
			if (target_y + center_y > center_y * 2.f) target_y = 0.f;
		}

		if (y < center_y)
		{
			target_y = y - center_y;
			target_y /= smooth;
			if (target_y + center_y < 0.f) target_y = 0.f;
		}
	}

	MouseController::Move_Mouse((int)target_x, (int)(target_y));
}

int playerHitbox = 68;
void AimAt(DWORD_PTR entity)
{
	uint64_t currentactormesh = KmDrv->Rpm<uint64_t>(entity + 0x310);
	auto rootHead = GetBoneWithRotation(currentactormesh, playerHitbox);
	Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
	if (rootHeadOut.y != 0 || rootHeadOut.y != 0)
	{
		move_to(rootHeadOut.x, rootHeadOut.y);
	}
}