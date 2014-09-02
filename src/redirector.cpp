
#include <wx/dcbuffer.h>
#include <rdlib/wxsup.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <rdlib/DataList.h>

#include "redirectorwindow.h"

#include "redirector_private.h"

// ----------------------------------------------------------------------------
// application class
// ----------------------------------------------------------------------------

class RedirectorApp: public wxApp
{
public:
    virtual bool OnInit();
};


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

class RedirectorFrame : public wxFrame
{
public:
    RedirectorFrame(int argc, wxChar **argv, const wxString& title, const wxPoint& pos, const wxSize& size);
    virtual ~RedirectorFrame();

    void OnNewServerClientConnection(wxCommandEvent &);
    void OnNewClientClientConnection(wxCommandEvent &);
    void OnNewServerServerConnection(wxCommandEvent &);
    void OnNewMultiClientConnection(wxCommandEvent &);
    void OnQuit(wxCommandEvent &);
    void OnTimer(wxTimerEvent &);
    void OnPaint(wxPaintEvent & event);

    DECLARE_EVENT_TABLE()

protected:
	void OnNewConnection(uint_t type);

	static void __closecallback(wxFrame *frame, void *context) {
		((RedirectorFrame *)context)->RemoveFrame(frame);
	}

	void RemoveFrame(wxFrame *frame) {
		m_childlist.Remove((uptr_t)frame);

		Refresh(false);
	}

private:
	ASocketServer m_server;
	wxFont m_font;
	wxTimer m_timer;
	ADataList m_childlist;
	wxPoint m_pos;
};

// ----------------------------------------------------------------------------
// menu item identifiers
// ----------------------------------------------------------------------------

enum
{
	ID_NewServerClientConnection = 1,
	ID_NewClientClientConnection,
	ID_NewServerServerConnection,
	ID_NewMultiClientConnection,
    ID_Quit,
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// application class
// ----------------------------------------------------------------------------

IMPLEMENT_APP(RedirectorApp)

bool RedirectorApp::OnInit()
{
#if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
#endif

    RedirectorFrame *frame = new RedirectorFrame (argc, argv,
												  _T(PRODUCT_NAME " - " VER_STRING),
												  wxPoint(200, 200), wxSize(540, 140));

    frame->Show (true);
    SetTopWindow (frame);

    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(RedirectorFrame, wxFrame)
    EVT_MENU (ID_NewServerClientConnection, RedirectorFrame::OnNewServerClientConnection)
    EVT_MENU (ID_NewClientClientConnection, RedirectorFrame::OnNewClientClientConnection)
    EVT_MENU (ID_NewServerServerConnection, RedirectorFrame::OnNewServerServerConnection)
    EVT_MENU (ID_NewMultiClientConnection, RedirectorFrame::OnNewMultiClientConnection)
    EVT_MENU (ID_Quit, RedirectorFrame::OnQuit)
	EVT_TIMER (0, RedirectorFrame::OnTimer)
    EVT_PAINT (RedirectorFrame::OnPaint)
END_EVENT_TABLE()

RedirectorFrame::RedirectorFrame(int argc, wxChar **argv, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size),
	m_font(36, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
	m_timer(this, 0),
	m_pos(pos)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	// create menu and items
    wxMenu *menuFile = new wxMenu;
    menuFile->Append (ID_NewServerClientConnection, _T("New Server-Client Connection\tAlt-N"));
    menuFile->Append (ID_NewClientClientConnection, _T("New Client-Client Connection"));
    menuFile->Append (ID_NewServerServerConnection, _T("New Server-Server Connection"));
    menuFile->Append (ID_NewMultiClientConnection,  _T("New Multi-Client Connection"));
    menuFile->Append (ID_Quit, _T("E&xit\tAlt-X"));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append (menuFile, _T("&Connection"));

    SetMenuBar (menuBar);

	// start timer to serve connections every 10ms
	m_timer.Start(10);

	int i;
	for (i = 1; i < argc; i++) {
		AString arg = _AString(argv[i]);
		AString stype, localaddr, localport, remoteaddr, remoteport;
		uint_t type = RedirectorWindowFrame::Type_Server_Client;

		arg.GetFieldNumber(":", 0, stype);
		arg.GetFieldNumber(":", 1, localaddr);
		arg.GetFieldNumber(":", 2, localport);
		arg.GetFieldNumber(":", 3, remoteaddr);
		arg.GetFieldNumber(":", 4, remoteport);

		if (localport.Empty())  localport  = "8080";
		if (remoteport.Empty()) remoteport = "8081";

		if		(stype == "sc") type = RedirectorWindowFrame::Type_Server_Client;
		else if (stype == "cc") type = RedirectorWindowFrame::Type_Client_Client;
		else if (stype == "ss") type = RedirectorWindowFrame::Type_Server_Server;
		else if (stype == "mc") type = RedirectorWindowFrame::Type_Multi_Client;

		RedirectorWindowFrame *frame = new RedirectorWindowFrame (this,
																  _wxString(arg),
																  m_pos, wxSize(480, 240),
																  m_server,
																  type,
																  _wxString(localaddr), (uint_t)localport,
																  _wxString(remoteaddr), (uint_t)remoteport,
																  &__closecallback, this);
		if (frame && frame->Valid()) {
			// connection(s) created okay -> show window and add frame to childlist
			frame->Show (true);
	
			m_childlist.Add((uptr_t)frame);
		}
		else {
			// connection(s) failed -> display error message
			wxString msg = _T("Unable to create connection");

			wxMessageDialog dlg(this, msg, _T("Connection Failure"), wxICON_ERROR | wxOK);
			dlg.ShowModal();

			if (frame) delete frame;
		}
	}
}

RedirectorFrame::~RedirectorFrame()
{
	RedirectorWindowFrame *frame;
	
	// for all children still operational, Destroy and then delete the frames
	while ((frame = (RedirectorWindowFrame *)m_childlist.Pop()) != NULL) {
		frame->Destroy();

		delete frame;
	}
}

void RedirectorFrame::OnNewConnection(uint_t type)
{
	wxString localaddrmsg  = _T("Enter local address");
	wxString localportmsg  = _T("Enter local port");
	wxString remoteaddrmsg = _T("Enter remote address");
	wxString remoteportmsg = _T("Enter remote port");
	wxString anymsg        = _T("\n\nLeave blank for any interface");
	wxString localaddr;
	wxString remoteaddr;
	wxString title;
	long localport  	   = 8080;
	long remoteport 	   = 8081;

	switch (type) {
		default:
		case RedirectorWindowFrame::Type_Server_Client:
			localaddrmsg  = _T("Enter server address") + anymsg;
			localportmsg  = _T("Enter server port");
			remoteaddrmsg = _T("Enter client address");
			remoteportmsg = _T("Enter client port");
			break;

		case RedirectorWindowFrame::Type_Client_Client:
			localaddrmsg  = _T("Enter client 1 address");
			localportmsg  = _T("Enter client 1 port");
			remoteaddrmsg = _T("Enter client 2 address");
			remoteportmsg = _T("Enter client 2 port");
			break;

		case RedirectorWindowFrame::Type_Multi_Client:
			localaddrmsg = _T("Enter server address") + anymsg;
			localportmsg = _T("Enter server port");
			break;

		case RedirectorWindowFrame::Type_Server_Server:
			localaddrmsg  = _T("Enter server 1 address") + anymsg;
			localportmsg  = _T("Enter server 1 port");
			remoteaddrmsg = _T("Enter server 2 address") + anymsg;
			remoteportmsg = _T("Enter server 2 port");
			break;
	}

	{
		wxTextEntryDialog dlg(this,
							  localaddrmsg, 
							  _T("New Connection"),
							  localaddr);

		if (dlg.ShowModal() == wxID_OK) {
			localaddr = dlg.GetValue();
		}
		else return;
	}

	{
		wxString tmp;

		tmp.Printf(_T("%ld"), localport);
		wxTextEntryDialog dlg(this,
							  localportmsg, 
							  _T("New Connection"),
							  tmp);
		
		if (dlg.ShowModal() == wxID_OK) {
			dlg.GetValue().ToLong(&localport);
		}
		else return;
	}

	if (type != RedirectorWindowFrame::Type_Multi_Client) {
		// don't need second address-port pair for multi-client
		{
			wxTextEntryDialog dlg(this,
								  remoteaddrmsg, 
								  _T("New Connection"),
								  remoteaddr);

			if (dlg.ShowModal() == wxID_OK) {
				remoteaddr = dlg.GetValue();
			}
			else return;
		}

		{
			wxString tmp;

			tmp.Printf(_T("%ld"), remoteport);
			wxTextEntryDialog dlg(this,
								  remoteportmsg,
								  _T("New Connection"),
								  tmp);
		
			if (dlg.ShowModal() == wxID_OK) {
				dlg.GetValue().ToLong(&remoteport);
			}
			else return;
		}
	}

	m_pos.x += 20;
	m_pos.y += 20;

	switch (type) {
		default:
		case RedirectorWindowFrame::Type_Server_Client:
			title.Printf(_T("Server-Client Connection from %s:%ld to %s:%ld"), 
						 localaddr.c_str(), localport,
						 remoteaddr.c_str(), remoteport);
			break;

		case RedirectorWindowFrame::Type_Client_Client:
			title.Printf(_T("Client-Client Connection from %s:%ld to %s:%ld"), 
						 localaddr.c_str(), localport,
						 remoteaddr.c_str(), remoteport);
			break;

		case RedirectorWindowFrame::Type_Multi_Client:
			title.Printf(_T("Multi-Client Server at %s:%ld"), 
						 localaddr.c_str(), localport);
			break;

		case RedirectorWindowFrame::Type_Server_Server:
			title.Printf(_T("Server-Server Connection from %s:%ld to %s:%ld"), 
						 localaddr.c_str(), localport,
						 remoteaddr.c_str(), remoteport);
			break;
	}

	// create frame to spawn connections
	RedirectorWindowFrame *frame = new RedirectorWindowFrame (this,
															  title,
															  m_pos, wxSize(480, 240),
															  m_server,
															  type,
															  localaddr, localport,
															  remoteaddr, remoteport,
															  &__closecallback, this);
	if (frame && frame->Valid()) {
		// connection(s) created okay -> show window and add frame to childlist
		frame->Show (true);
	
		m_childlist.Add((uptr_t)frame);
	}
	else {
		// connection(s) failed -> display error message
		wxString msg = _T("Unable to create connection");

		wxMessageDialog dlg(this, msg, _T("Connection Failure"), wxICON_ERROR | wxOK);
		dlg.ShowModal();

		if (frame) delete frame;
	}

	Refresh(false);
}

void RedirectorFrame::OnNewServerClientConnection(wxCommandEvent &)
{
	OnNewConnection(RedirectorWindowFrame::Type_Server_Client);

	Refresh(false);
}

void RedirectorFrame::OnNewClientClientConnection(wxCommandEvent &)
{
	OnNewConnection(RedirectorWindowFrame::Type_Client_Client);

	Refresh(false);
}

void RedirectorFrame::OnNewServerServerConnection(wxCommandEvent &)
{
	OnNewConnection(RedirectorWindowFrame::Type_Server_Server);

	Refresh(false);
}

void RedirectorFrame::OnNewMultiClientConnection(wxCommandEvent &)
{
	OnNewConnection(RedirectorWindowFrame::Type_Multi_Client);

	Refresh(false);
}

void RedirectorFrame::OnQuit (wxCommandEvent &)
{
	Close(true);
}

void RedirectorFrame::OnTimer(wxTimerEvent &)
{
	uint_t i;

	// process all connections with 0 timeout up to 16 times
	// (to prevent endless loop if a loop occurs)
	for (i = 0; (i < 16) && (m_server.Process(0) > 0); i++) ;
}

void RedirectorFrame::OnPaint (wxPaintEvent &)
{
	wxAutoBufferedPaintDC dc(this);

	dc.SetBackground(wxBrush(wxColour(255, 255, 255)));
	dc.Clear();

	dc.SetFont(m_font);
	dc.SetTextBackground(wxColour(255, 255, 255));

	wxSize sz = dc.GetTextExtent(_T("0"));
	int sz_h = sz.GetHeight();
	wxPoint pos(5 + sz_h + 5, 5);
	wxString str;

	if (m_childlist.Count() == 1) {
		str.Printf(_T("%u connection"), m_childlist.Count());
	}
	else {
		str.Printf(_T("%u connections"), m_childlist.Count());
	}

	dc.SetTextForeground(wxColour(0, 0, 0));
	dc.DrawText(str, pos.x, pos.y); pos.y += sz_h + 10;
	
	dc.SetFont(wxNullFont);
}
