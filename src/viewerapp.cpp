
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/wx.h>
#include <wx/image.h>

#include <rdlib/wxsup.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include "viewer.h"
#include "viewer_version.h"

class ViewerApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int  OnExit();
};

IMPLEMENT_APP(ViewerApp)

bool ViewerApp::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler);

    Viewer *frame = new Viewer(argc, argv);
    frame->Show(true);
    SetTopWindow(frame);

    return true;
}

int ViewerApp::OnExit()
{
	return wxApp::OnExit();
}

