#pragma once

class EditorHookWindow : public ISingleton <EditorHookWindow>
{
public:
	EditorHookWindow();
	~EditorHookWindow();

	void	PumpEvents(void);

private:
	LRESULT					WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK	_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

	void	OnButtonHit(void);

	HWND			m_window;
	volatile bool	m_done;

	HWND	m_button;
	HWND	m_editText, m_editText2;
};
