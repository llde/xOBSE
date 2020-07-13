#include "EditorHookWindow.h"
#include "common/IFileStream.h"
#include "obse/Utilities.h"

EditorHookWindow::EditorHookWindow()
:m_window(NULL), m_button(NULL), m_editText(NULL), m_editText2(NULL)
{
	// register our window class
	WNDCLASS	windowClass =
	{
		0,								// style
		_WindowProc,					// window proc
		0, 0,							// no extra memory required
		GetModuleHandle(NULL),			// instance
		NULL, NULL,						// icon, cursor
		(HBRUSH)(COLOR_BACKGROUND + 1),	// background brush
		NULL,							// menu name
		"EditorHookWindow"				// class name
	};

	ATOM	classAtom = RegisterClass(&windowClass);
	ASSERT(classAtom);

	CreateWindow(
		(LPCTSTR)classAtom,				// class
		"OBSE",							// name
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,	// style
		0, 0,							// x y
		300, 300,						// width height
		NULL,							// parent
		NULL,							// menu
		GetModuleHandle(NULL),			// instance
		(LPVOID)this);
}

EditorHookWindow::~EditorHookWindow()
{
	if(m_window)
		DestroyWindow(m_window);
}

void EditorHookWindow::PumpEvents(void)
{
	MSG	msg;

	m_done = false;

	while(!m_done)
	{
		BOOL	result = GetMessage(&msg, m_window, 0, 0);
		if(result == -1)
		{
			_MESSAGE("message pump error");
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if(result == 0)
			break;
	}
}

LRESULT EditorHookWindow::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
//	_MESSAGE("windowproc: %08X %08X %08X", msg, wParam, lParam);

	switch(msg)
	{
		case WM_CREATE:
			m_button = CreateWindow(
				"BUTTON",
				"Push Button",	// receive bacon
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			m_editText = CreateWindow(
				"EDIT",
				NULL,
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				0, 30,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			m_editText2 = CreateWindow(
				"EDIT",
				NULL,
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				110, 30,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);

			ASSERT(m_button && m_editText);
			break;

		case WM_COMMAND:
		{
			HWND	source = (HWND)lParam;
			if(source == m_button)
			{
				OnButtonHit();
			}
		}
		break;

		case WM_DESTROY:
			m_done = true;
			break;
	};

	return DefWindowProc(m_window, msg, wParam, lParam);
}

LRESULT EditorHookWindow::_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EditorHookWindow	* _this = NULL;

	if(msg == WM_CREATE)
	{
		CREATESTRUCT	* info = (CREATESTRUCT *)lParam;
		_this = (EditorHookWindow *)info->lpCreateParams;

		SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)_this);

		_this->m_window = window;
	}
	else
	{
		_this = (EditorHookWindow *)GetWindowLongPtr(window, GWLP_USERDATA);
	}

	LRESULT	result;

	if(_this)
		result = _this->WindowProc(msg, wParam, lParam);
	else
		result = DefWindowProc(window, msg, wParam, lParam);

	return result;
}

typedef void * (* _LookupFormByID)(UInt32 id);
const _LookupFormByID LookupFormByID = (_LookupFormByID)0x00495EF0;

#if 0
#pragma warning (push)
#pragma warning (disable : 4200)
struct RTTIType
{
	void	* typeInfo;
	UInt32	pad;
	char	name[0];
};

struct RTTILocator
{
	UInt32		sig, offset, cdOffset;
	RTTIType	* type;
};
#pragma warning (pop)

// use the RTTI information to return an object's class name
const char * GetObjectClassName(void * objBase)
{
	const char	* result = "<no rtti>";

	__try
	{
		void		** obj = (void **)objBase;
		RTTILocator	** vtbl = (RTTILocator **)obj[0];
		RTTILocator	* rtti = vtbl[-1];
		RTTIType	* type = rtti->type;

		// starts with ,?
		if((type->name[0] == '.') && (type->name[1] == '?'))
		{
			// is at most 100 chars long
			for(UInt32 i = 0; i < 100; i++)
			{
				if(type->name[i] == 0)
				{
					// remove the .?AV
					result = type->name + 4;
					break;
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// return the default
	}

	return result;
}
#endif

void EditorHookWindow::OnButtonHit(void)
{
	char	text[256];
	GetWindowText(m_editText, text, sizeof(text));

	char	comment[256];
	GetWindowText(m_editText2, comment, sizeof(comment));

	UInt32	id;
	if(sscanf_s(text, "%x", &id) == 1)
	{
		void	* ptr = LookupFormByID(id);
		
		sprintf_s(text, sizeof(text), "%08X = %08X (%s)", id, (UInt32)ptr, GetObjectClassName(ptr));
		_MESSAGE("%s", text);

		MessageBox(m_window, text, "receive bacon", MB_OK);

		static int idx = 0;
		char	fileName[256];
		if(comment[0])
			sprintf_s(fileName, sizeof(fileName), "mem%08X_%08X_%08X_%s", idx, id, (UInt32)ptr, comment);
		else
			sprintf_s(fileName, sizeof(fileName), "mem%08X_%08X_%08X", idx, id, (UInt32)ptr);
		idx++;

		IFileStream	dst;
		if(dst.Create(fileName))
			dst.WriteBuf(ptr, 0x200);
	}
	else
	{
		MessageBox(m_window, "couldn't read text box", "receive bacon", MB_OK);
	}
}
