
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/dcbuffer.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include "viewer.h"
#include "viewer_version.h"

BEGIN_EVENT_TABLE(Viewer, wxFrame)
    EVT_PAINT(Viewer::OnPaint)
	EVT_SIZE(Viewer::OnSize)
	EVT_MOVE(Viewer::OnMove)
	EVT_ERASE_BACKGROUND(Viewer::OnEraseBackground)
	EVT_MAXIMIZE(Viewer::OnMaximize)
	EVT_LEFT_DCLICK(Viewer::OnMouseLeftDoubleClick)
    EVT_LEFT_DOWN(Viewer::OnMouseLeftDown)
    EVT_LEFT_UP(Viewer::OnMouseLeftUp)
    EVT_RIGHT_DOWN(Viewer::OnMouseRightDown)
    EVT_RIGHT_UP(Viewer::OnMouseRightUp)
    EVT_MOTION(Viewer::OnMouseMove)
    EVT_MOUSEWHEEL(Viewer::OnMouseWheel)
    EVT_KEY_DOWN(Viewer::OnKeyDown)
	EVT_KEY_UP(Viewer::OnKeyUp)
END_EVENT_TABLE();

Viewer::Viewer(int argc, wxChar **argv) : wxFrame((wxFrame *)NULL,
												  wxID_ANY,
												  _T(PRODUCT_NAME " - " VER_STRING)),
										  settings(PRODUCT_NAME),
										  font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
										  drag_action(Drag_Action_None)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	linelist.SetDestructor(&__deleteline);

	view.transform.xy  	= (double)settings.Get("view_xy", "0.0");
	view.transform.yz  	= (double)settings.Get("view_yz", "0.0");
	view.transform.xz  	= (double)settings.Get("view_xz", "0.0");
	view.transform.per 	= (double)settings.Get("view_pers", "1000.0");
	view.transform.zoom = (double)settings.Get("view_zoom", "1.0");

	view.xscale = view.yscale = view.scale = 1.0;

	SetBackgroundColour(wxColour(_T("#000000")));

	int i;
	for (i = 1; i < argc; i++) {
		AString *filename;

		if ((filename = new AString(_AString(wxString(argv[i])))) != NULL) {
			filelist.Add(filename);
		}
	}
	filename = AString::Cast(filelist.First());
	LoadFile();
}
										  
Viewer::~Viewer()
{
}

void Viewer::Clear()
{
	linelist.DeleteList();

	bb_corner1 = bb_corner2 = A3DPoint();
}

void Viewer::AddLine(const AString& str)
{
	LINE *line;

	if ((line = new LINE) != NULL) {
		AString _pt1 = str.Word(0).DeQuotify().SearchAndReplace(",", " ");
		AString _pt2 = str.Word(1).DeQuotify().SearchAndReplace(",", " ");
		A3DPoint pt1((double)_pt1.Word(0), (double)_pt1.Word(1), (double)_pt1.Word(2));
		A3DPoint pt2((double)_pt2.Word(0), (double)_pt2.Word(1), (double)_pt2.Word(2));

		line->pt1 	= pt1;
		line->pt2 	= pt2;
		line->index = (uint_t)str.Word(2);
		line->pen.SetColour(_wxString(str.Word(3)));

		if (linelist.Count() == 0) {
			bb_corner1.x = MIN(pt1.x, pt2.x);
			bb_corner1.y = MIN(pt1.y, pt2.y);
			bb_corner1.z = MIN(pt1.z, pt2.z);
			bb_corner2.x = MAX(pt1.x, pt2.x);
			bb_corner2.y = MAX(pt1.y, pt2.y);
			bb_corner2.z = MAX(pt1.z, pt2.z);
		}
		else {
			bb_corner1.x = MIN(bb_corner1.x, pt1.x);
			bb_corner1.x = MIN(bb_corner1.x, pt2.x);
			bb_corner1.y = MIN(bb_corner1.y, pt1.y);
			bb_corner1.y = MIN(bb_corner1.y, pt2.y);
			bb_corner1.z = MIN(bb_corner1.z, pt1.z);
			bb_corner1.z = MIN(bb_corner1.z, pt2.z);
			bb_corner2.x = MAX(bb_corner2.x, pt1.x);
			bb_corner2.x = MAX(bb_corner2.x, pt2.x);
			bb_corner2.y = MAX(bb_corner2.y, pt1.y);
			bb_corner2.y = MAX(bb_corner2.y, pt2.y);
			bb_corner2.z = MAX(bb_corner2.z, pt1.z);
			bb_corner2.z = MAX(bb_corner2.z, pt2.z);
		}

		linelist.Add((uptr_t)line);
	}
}

bool Viewer::LoadFile()
{
	AStdFile fp;
	bool success = false;

	if (!filename) {
		debug("No files!\n");
		return false;
	}

	if (fp.open(*filename)) {
		AString line;
		uint_t n = 0;

		SetTitle(wxString::Format(_T(PRODUCT_NAME " - " VER_STRING " - %s"), _wxString(*filename).c_str()));

		Clear();

		while (line.ReadLn(fp) >= 0) {
			if (line.Valid() && (line[0] != '#')) {
				AddLine(line);

				n++;
			}
		}

		fp.close();

		view.translation = -.5 * (bb_corner1 + bb_corner2);
		view_index = 0;

		Refresh(false);

		success = true;
	}
	else {
		debug("Failed to open file '%s' for reading\n", filename->str());

		wxMessageDialog dlg(this,
							wxString::Format(_T("Failed to open file '%s'"), _wxString(*filename).c_str()),
							_T("File open failed!"), wxICON_ERROR | wxOK);
			
		dlg.ShowModal();

		AString *filename1;
		if ((filename1 = filename->Next()) == NULL) filename1 = filename->Prev();

		filelist.Remove(filename);
		delete filename;
		filename = filename1;
	}

	return success;
}

void Viewer::OnSize(wxSizeEvent &)
{
	Refresh(false);
}

void Viewer::OnMove(wxMoveEvent &)
{
}

void Viewer::OnMaximize(wxMaximizeEvent &)
{
	Refresh(false);
}

void Viewer::OnEraseBackground(wxEraseEvent &)
{
}

void Viewer::OnKeyUp(wxKeyEvent &)
{
}

void Viewer::OnKeyDown(wxKeyEvent &)
{
}

void Viewer::OnMouseLeftDoubleClick(wxMouseEvent &)
{
}

void Viewer::OnMouseLeftDown(wxMouseEvent & event)
{
	CaptureMouse();

	wxPoint	mousepos = wxPoint(event.m_x, event.m_y);

	drag_start_view     = view;
	drag_start_view.translation = A3DPoint();
	drag_start_pos      = view.translation * drag_start_view;
	drag_start_mousepos = mousepos;
	drag_shift_down     = event.ShiftDown();
	drag_move_offset    = wxPoint((int)(drag_start_pos.x + view.cx),
								  (int)(drag_start_pos.y + view.cy)) - mousepos;
	drag_action         = Drag_Action_Rotate;

	Refresh(false);
}

void Viewer::OnMouseLeftUp(wxMouseEvent &)
{
	ReleaseMouse();

	drag_action = Drag_Action_None;

	settings.Set("view_xy", AString("%0.3").Arg(view.transform.xy));
	settings.Set("view_yz", AString("%0.3").Arg(view.transform.yz));
	settings.Set("view_xz", AString("%0.3").Arg(view.transform.xz));

	Refresh(false);
}

void Viewer::OnMouseRightDown(wxMouseEvent & event)
{
	CaptureMouse();

	wxPoint	mousepos = wxPoint(event.m_x, event.m_y);

	drag_start_view     = view;
	drag_start_view.translation = A3DPoint();
	drag_start_pos      = view.translation * drag_start_view;
	drag_start_mousepos = mousepos;
	drag_shift_down     = event.ShiftDown();
	drag_move_offset    = wxPoint((int)(drag_start_pos.x + view.cx),
								  (int)(drag_start_pos.y + view.cy)) - mousepos;
	drag_action         = Drag_Action_Move;

	Refresh();
}

void Viewer::OnMouseRightUp(wxMouseEvent &)
{
	ReleaseMouse();

	drag_action = Drag_Action_None;

	Refresh(false);
}

void Viewer::OnMouseMove(wxMouseEvent & event)
{
	wxPoint	mousepos = wxPoint(event.m_x, event.m_y);

	switch (drag_action) {
		case Drag_Action_None:
			break;

		case Drag_Action_Rotate: {
			if (event.ShiftDown() != drag_shift_down) {
				drag_start_view     = view;
				drag_start_mousepos = mousepos;
				drag_shift_down     = event.ShiftDown();
			}

			double offset_x = (double)(mousepos.x - drag_start_mousepos.x) / 600.0;
			double offset_y = (double)(mousepos.y - drag_start_mousepos.y) / 600.0;

			if (drag_shift_down) {
				view.transform.xz = drag_start_view.transform.xz + offset_x * 360.0;
				view.transform.xy = drag_start_view.transform.xy + offset_y * 360.0;
			}
			else {
				view.transform.xz = drag_start_view.transform.xz + offset_x * 360.0;
				view.transform.yz = drag_start_view.transform.yz + offset_y * 360.0;
			}

			while (view.transform.xy >= 180.0) view.transform.xy -= 360.0;
			while (view.transform.yz >= 180.0) view.transform.yz -= 360.0;
			while (view.transform.xz >= 180.0) view.transform.xz -= 360.0;
			while (view.transform.xy < -180.0) view.transform.xy += 360.0;
			while (view.transform.yz < -180.0) view.transform.yz += 360.0;
			while (view.transform.xz < -180.0) view.transform.xz += 360.0;

			Refresh();
			break;
		}

		case Drag_Action_Move: {
			A3DPoint pos((double)(mousepos.x + drag_move_offset.x - view.cx), 
						 (double)(mousepos.y + drag_move_offset.y - view.cy),
						 drag_start_pos.z);
			
			A3DView tempview = view;
			tempview.translation = A3DPoint();
			view.translation = pos / tempview;
			
			Refresh();
			break;
		}
	}
}

void Viewer::OnMouseWheel(wxMouseEvent & event)
{
	if (event.ControlDown()) {
		double movement = (double)event.GetWheelRotation() / 120.0;
		bool   load = false;

		if (movement < 0.0) {
			if (filename && filename->Next()) {
				filename = filename->Next();
				load = true;
			}
		}

		if (movement > 0.0) {
			if (filename && filename->Prev()) {
				filename = filename->Prev();
				load = true;
			}
		}

		if (load) LoadFile();
	}
	else if (event.ShiftDown()) {
		double movement = (double)event.GetWheelRotation() / 120.0;

		if (movement < 0.0) {
			if (view_index) {
				view_index--;
				//printf("View index = %u\n", view_index);
			}
		}
		if (movement > 0.0) {
			view_index++;
			//printf("View index = %u\n", view_index);
		}

		Refresh(false);
	}
	else {
		view.transform.zoom *= pow(2.0, .1 * (double)event.GetWheelRotation() / 120.0);
		view.transform.zoom = LIMIT(view.transform.zoom, 1.0e-5, 1.0e3);

		settings.Set("view_zoom", AString("%0.4").Arg(view.transform.zoom));

		Refresh();
	}
}

void Viewer::OnPaint(wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);		// automatically double buffering!!

	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();
	
	wxSize sz = GetClientSize();

	view.cx = .5 * (double)sz.GetWidth();
	view.cy = .5 * (double)sz.GetHeight();

	double dist = (bb_corner2 - bb_corner1).mod();
	if (dist > 0.0) view.scale = (double)MIN(sz.GetWidth(), sz.GetHeight()) / dist;
	view.transform.per = dist * 2.0;

	wxRect rect(-32768, -32768, 65535, 65535);
	const LINE **lines = (const LINE **)linelist.List();
	uint_t i, n = linelist.Count();
	for (i = 0; i < n; i++) {
		if (!view_index || (lines[i]->index == (view_index - 1))) {
			A3DPoint pt1 = lines[i]->pt1 * view;
			A3DPoint pt2 = lines[i]->pt2 * view;
			wxPoint  p1((int)pt1.x, (int)pt1.y);
			wxPoint  p2((int)pt2.x, (int)pt2.y);
		
			if (rect.Contains(p1) && rect.Contains(p2)) {
				dc.SetPen(lines[i]->pen);
			
				dc.DrawLine((int)pt1.x, (int)pt1.y,
							(int)pt2.x, (int)pt2.y);
			}
		}
	}

	dc.SetPen(wxNullPen);
}
