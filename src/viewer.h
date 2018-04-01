#ifndef __VIEWER__
#define __VIEWER__

#include <wx/wx.h>
#include <rdlib/wxsup.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <rdlib/strsup.h>
#include <rdlib/DataList.h>
#include <rdlib/3DTrans.h>
#include <rdlib/SettingsHandler.h>

class Viewer : public wxFrame {
public:
	Viewer(int argc, wxChar **argv);
	virtual ~Viewer();

protected:
	void OnTimer(wxTimerEvent &);
	void OnPaint(wxPaintEvent &);
	void OnSize(wxSizeEvent &sizeevent);
	void OnMove(wxMoveEvent &);
	void OnMaximize(wxMaximizeEvent &);
	void OnEraseBackground(wxEraseEvent &);

	void OnMouseLeftDoubleClick(wxMouseEvent & event);
	void OnMouseLeftDown(wxMouseEvent & event);
	void OnMouseLeftUp(wxMouseEvent & event);
	void OnMouseRightDown(wxMouseEvent & event);
	void OnMouseRightUp(wxMouseEvent & event);
	void OnMouseMove(wxMouseEvent & event);
	void OnMouseWheel(wxMouseEvent & event);

	void OnKeyUp(wxKeyEvent& evt);
	void OnKeyDown(wxKeyEvent& evt);

    DECLARE_EVENT_TABLE();

	typedef struct {
		A3DPoint pt1, pt2;
		uint_t   index;
		wxPen    pen;
	} LINE;

	static void __deleteline(uptr_t item, void *context) {
		(void)context;
		delete (LINE *)item;
	}

	void Clear();
	void AddLine(const AString& str);
	bool LoadFile();

	enum {
		Drag_Action_None = 0,
		Drag_Action_Rotate,
		Drag_Action_Move,
	};

protected:
	ASettingsHandler settings;
	ADataList linelist;
	AList     filelist;
	AString   *filename;
	wxFont    font;
	A3DView	  view;
	A3DPoint  bb_corner1, bb_corner2;
	A3DView	  drag_start_view;
	A3DPoint  drag_start_pos;
	wxPoint	  drag_start_mousepos;
	wxPoint   drag_move_offset;
	bool	  drag_shift_down;
	uint_t	  drag_action;
	uint_t	  view_index;
};

#endif
