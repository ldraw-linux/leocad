#ifndef _GLWINDOW_H_
#define _GLWINDOW_H_

enum LC_CURSOR_TYPE
{
	LC_CURSOR_DEFAULT,
	LC_CURSOR_BRICK,
	LC_CURSOR_LIGHT,
	LC_CURSOR_SPOTLIGHT,
	LC_CURSOR_CAMERA,
	LC_CURSOR_SELECT,
	LC_CURSOR_SELECT_GROUP,
	LC_CURSOR_MOVE,
	LC_CURSOR_ROTATE,
	LC_CURSOR_ROTATEX,
	LC_CURSOR_ROTATEY,
	LC_CURSOR_DELETE,
	LC_CURSOR_PAINT,
	LC_CURSOR_ZOOM,
	LC_CURSOR_ZOOM_REGION,
	LC_CURSOR_PAN,
	LC_CURSOR_ROLL,
	LC_CURSOR_ROTATE_VIEW,
	LC_CURSOR_COUNT
};

struct lcInputState
{
	int x;
	int y;
	bool Control;
	bool Shift;
	bool Alt;
};

class GLWindow
{
public:
	GLWindow();
	virtual ~GLWindow();

	bool CreateFromWindow(void* Data);
	bool CreateFromBitmap(void* Data);
	void* GetExtensionAddress(const char* FunctionName);

	bool MakeCurrent();
	void Redraw();
	void CaptureMouse();
	void ReleaseMouse();
	void SetCursor(LC_CURSOR_TYPE Cursor);

	virtual void OnDraw() { }
	virtual void OnSize(int cx, int cy)
	{ mWidth = cx; mHeight = cy; }
	virtual void OnInitialUpdate();
	virtual void OnUpdateCursor() { }
	virtual void OnLeftButtonDown() { }
	virtual void OnLeftButtonUp() { }
	virtual void OnLeftButtonDoubleClick() { }
	virtual void OnMiddleButtonDown() { }
	virtual void OnMiddleButtonUp() { }
	virtual void OnRightButtonDown() { }
	virtual void OnRightButtonUp() { }
	virtual void OnMouseMove() { }
	virtual void OnMouseWheel(float Direction) { }

	lcInputState mInputState;
	int mWidth;
	int mHeight;
	int mCursorType;
	void* mWidget;
};

#endif // _GLWINDOW_H_
