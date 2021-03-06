#pragma once

#include <string>
#include <queue>

#include <Renderer/Backend/Common.hpp>
#include <Renderer/Backend/Window/Event.hpp>
#include <Legacy/Math/Vec.hpp>
#include <Legacy/Math/Vec2.hpp>

namespace WindowStyle
{
	enum window_style_t
	{
		Base = 1,
		Resize = 2,
		Close = 4,
		Borderless = 8,
		Fullscreen = 16
	};

	inline window_style_t operator|(window_style_t lft, window_style_t rht)
	{
		return (window_style_t)((int)lft | (int)rht);
	}
}

TRE_NS_START

class RENDERER_API Window
{
public:
	Window(uint32 width = 800, uint32 height = 600, const std::string& title = "Window", WindowStyle::window_style_t style = WindowStyle::Close);
	~Window();

	const Vec2<uint32>& getSize() const;
	const Vec2<int32>& getPosition() const;
	const Vec2<int32>& getMousePosition() const;

	void setWindowPosition(int x, int y);
	void setWindowSize(uint32 width, uint32 height);

	bool isOpen() const;
	bool hasFocus() const;

	void setWindowTitle(const std::string& title);
	void setVisible(bool visible);
	void Close();
	bool getEvent(Event& ev);
	bool isMouseButtonDown(MouseButton::mouse_button_t button) const;
	bool isKeyDown(Key::key_t key) const;

	void PollEvents();

	void WaitEvents();

	void Present();

	void* GetNativeHandle();

#if defined(OS_LINUX)
	::Window GetWindowHandle() const { return window;}
#endif

public:
	Vec2<uint32> windowSize;
	Vec2<int32> windowPosition;
	Vec2<int32> mousePosition;

	bool open, focus;
	std::queue<Event> events;
	bool mouse[3];
	bool keys[100];

#if defined( OS_WINDOWS )
	HWND window;
	DWORD style;

	LRESULT WindowEvent(UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowEventHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined( OS_LINUX )
	::Window window;
	Display* display;
	Atom close;
	bool fullscreen;
	int screen;
	int oldVideoMode;

	void EnableFullscreen(bool enabled, int width = 0, int height = 0);
	void WindowEvent(const XEvent& event);
	static Bool CheckEvent(Display*, XEvent* event, XPointer userData);
#endif		

	Window(const Window&);
	const Window& operator=(const Window&);

	FORCEINLINE Key::key_t TranslateKey(uint code);
};

TRE_NS_END
