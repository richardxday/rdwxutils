
#include <wx/dcbuffer.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <rdlib/DateTime.h>

#include "redirectorwindow.h"

enum
{
	ID_ToggleDebug = 1,
	ID_ToggleTimestamp,
    ID_Close,
};

BEGIN_EVENT_TABLE(RedirectorWindowFrame, wxFrame)
	EVT_MENU (ID_ToggleDebug, RedirectorWindowFrame::OnToggleDebug)
	EVT_MENU (ID_ToggleTimestamp, RedirectorWindowFrame::OnToggleTimestamp)
	EVT_MENU (ID_Close, RedirectorWindowFrame::OnClose)
	EVT_TIMER (0, RedirectorWindowFrame::OnTimer)
    EVT_PAINT (RedirectorWindowFrame::OnPaint)
END_EVENT_TABLE()

RedirectorWindowFrame::RedirectorWindowFrame(wxWindow *parent,
											 const wxString& title, const wxPoint& pos, const wxSize& size,
											 ASocketServer& server, uint_t type,
											 const wxString& localaddr, uint_t localport,
											 const wxString& remoteaddr, uint_t remoteport,
											 void (*closecallback)(wxFrame *frame, void *context), void *closecontext) : wxFrame(parent, wxID_ANY, title, pos, size),
	m_server(server),
	m_timer(this, 0),
	m_font(36, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
	m_remoteaddr(remoteaddr),
	m_remoteport(remoteport),
	m_closecallback(closecallback),
	m_closecontext(closecontext),
	m_rxbytes(0),
	m_txbytes(0),
	m_last_rxbytes(0),
	m_last_txbytes(0),
	m_outputdebug(false),
    m_outputtimestamp(true)    
{
	// window graphics update rate = 100ms
	m_timer.Start(100);

	switch (type) {
		case Type_Server_Client: {
			ConnectionContext *context;

			if ((context = new ConnectionContext) != NULL) {
				context->m_frame = this;
				context->m_destsocket = -1;
				context->m_total = NULL;

				// create server that will create client connections
				int socket;
				if ((socket = m_server.CreateHandler(ASocketServer::Type_Server,
													 (localaddr.Len() > 0) ? _AString(localaddr).str() : NULL,
													 localport,
													 &__connectionhandler,
													 &__readhandler,
													 NULL,
													 &__destructor,
													 NULL,
													 context)) >= 0) {
					m_socketlist.Add(socket);
				}
				else
                {
                    wxString msg;
                    msg.Printf(_T("Unable to create a server on %s:%u"), localaddr.c_str(), localport);

                    wxMessageDialog dlg(this, msg, _T("Server Creation Failure"), wxICON_ERROR | wxOK);
                    dlg.ShowModal();
                    
                    delete context;
                }
			}
			break;
		}

		case Type_Client_Client: {
			ConnectionContext *context1, *context2;

			context1 = new ConnectionContext;
			context2 = new ConnectionContext;

			if (context1 && context2) {
				int socket1, socket2;

				context1->m_frame = this;
				context2->m_frame = this;
				context1->m_total = &m_rxbytes;
				context2->m_total = &m_txbytes;

				// create TWO client connections and connect them to each other
				socket2 = m_server.CreateHandler(ASocketServer::Type_Client,
												 _AString(localaddr).str(),
												 localport,
												 NULL,
												 &__readhandler,
												 NULL,
												 &__destructor,
												 NULL,
												 context1);

				socket1 = m_server.CreateHandler(ASocketServer::Type_Client,
												 _AString(remoteaddr).str(),
												 remoteport,
												 NULL,
												 &__readhandler,
												 NULL,
												 &__destructor,
												 NULL,
												 context2);

				if ((socket1 >= 0) &&
					(socket2 >= 0)) {
					context1->m_destsocket = socket1;
					context2->m_destsocket = socket2;

					m_socketlist.Add(context1->m_destsocket);
					m_socketlist.Add(context2->m_destsocket);
				}
				else {
					if (socket1 >= 0) m_server.DeleteHandler(socket1);
					else
                    {
                        wxString msg;
                        msg.Printf(_T("Unable to create a server on %s:%u"), remoteaddr.c_str(), remoteport);
                        
                        wxMessageDialog dlg(this, msg, _T("Server Creation Failure"), wxICON_ERROR | wxOK);
                        dlg.ShowModal();
                        
                        delete context2;
                    }
					if (socket2 >= 0) m_server.DeleteHandler(socket2);
                    else
                    {
                        wxString msg;
                        msg.Printf(_T("Unable to create a server on %s:%u"), localaddr.c_str(), localport);
                        
                        wxMessageDialog dlg(this, msg, _T("Server Creation Failure"), wxICON_ERROR | wxOK);
                        dlg.ShowModal();
                        
                        delete context2;
                    }
                }
			}
			else {
				if (context1) delete context1;
				if (context2) delete context2;
			}
			break;
		}

		case Type_Server_Server: {
			MultiConnectionContext *context1;
			MultiConnectionContext *context2;

			context1 = new MultiConnectionContext;
			context2 = new MultiConnectionContext;

			if (context1 && context2) {
				// create TWO servers that will allow clients connected to them to talk to each other

				context1->m_frame = this;
				context1->m_total = &m_rxbytes;
				context1->m_partner = context2;

				context2->m_frame = this;
				context2->m_total = &m_txbytes;
				context2->m_partner = context1;

				int socket1, socket2;
				socket1 = m_server.CreateHandler(ASocketServer::Type_Server,
												 (localaddr.Len() > 0) ? _AString(localaddr).str() : NULL,
												 localport,
												 &__server_connectionhandler,
												 &__server_readhandler,
												 NULL,
												 &__server_destructor,
												 NULL,
												 context1);
				socket2 = m_server.CreateHandler(ASocketServer::Type_Server,
												 (remoteaddr.Len() > 0) ? _AString(remoteaddr).str() : NULL,
												 remoteport,
												 &__server_connectionhandler,
												 &__server_readhandler,
												 NULL,
												 &__server_destructor,
												 NULL,
												 context2);

				if ((socket1 >= 0) && (socket2 >= 0)) {
					m_socketlist.Add(socket1);
					m_socketlist.Add(socket2);
				}
				else {
					if (socket1 >= 0) m_server.DeleteHandler(socket1);
					if (socket2 >= 0) m_server.DeleteHandler(socket2);
				}
			}
			else {
				if (context1) delete context1;
				if (context2) delete context2;
			}
			break;
		}

		case Type_Multi_Client: {
			MultiConnectionContext *context;

			// create server that multiple clients can connect to
			if ((context = new MultiConnectionContext) != NULL) {
				context->m_frame = this;
				context->m_total = &m_rxbytes;
				context->m_partner = NULL;

				int socket;
				if ((socket = m_server.CreateHandler(ASocketServer::Type_Server,
													 (localaddr.Len() > 0) ? _AString(localaddr).str() : NULL,
													 localport,
													 &__multi_connectionhandler,
													 &__multi_readhandler,
													 NULL,
													 &__multi_destructor,
													 NULL,
													 context)) >= 0) {
					m_socketlist.Add(socket);
				}
			}
			break;
		}
	}
			
	m_menu = new wxMenu;
    m_menu->AppendCheckItem(ID_ToggleDebug, _T("Output &Debug\tAlt-D"));
	m_menu->Check(ID_ToggleDebug, m_outputdebug);
    m_menu->AppendCheckItem(ID_ToggleTimestamp, _T("Output &Timestamp\tAlt-T"));
	m_menu->Check(ID_ToggleTimestamp, m_outputtimestamp);
    m_menu->Append(ID_Close, _T("C&lose\tAlt-L"));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append (m_menu, _T("&Connection"));

    SetMenuBar (menuBar);
}

RedirectorWindowFrame::~RedirectorWindowFrame()
{
	while (m_socketlist.Count() > 0) {
		int socket = m_socketlist.Pop();
		m_server.DeleteHandler(socket);
	}

	m_server.Process(0);

	if (m_closecallback) (*m_closecallback)(this, m_closecontext);
}

void RedirectorWindowFrame::OnToggleDebug(wxCommandEvent & event)
{
	(void)event;
	m_outputdebug = m_menu->IsChecked(ID_ToggleDebug);
}

void RedirectorWindowFrame::OnToggleTimestamp(wxCommandEvent & event)
{
	(void)event;
	m_outputtimestamp = m_menu->IsChecked(ID_ToggleTimestamp);
}

void RedirectorWindowFrame::OnClose(wxCommandEvent & event)
{
	(void)event;
	Close(true);
}

void RedirectorWindowFrame::connectionhandler(ASocketServer *server, int socket, ConnectionContext *context)
{
	ConnectionContext *context1, *context2;

	context1 = new ConnectionContext;
	context2 = new ConnectionContext;

	if (context1 && context2) {
        int socket1;
        
		*context1 = *context;
		*context2 = *context;

		m_socketlist.Add(socket);

		context1->m_total = &m_txbytes;
		server->SetContext(socket, context1);

		context2->m_destsocket = socket;
		context2->m_total = &m_rxbytes;

		if ((socket1 = server->CreateHandler(ASocketServer::Type_Client,
                                             _AString(m_remoteaddr),
                                             m_remoteport,
                                             NULL,
                                             &__readhandler,
                                             NULL,
                                             &__destructor,
                                             NULL,
                                             context2)) >= 0) {
            log(server, socket, AString("new connection to %s").Arg(GetIPAddress(server, socket1)));

            debug("Opened connection to %s:%u with socket %d\n", _AString(m_remoteaddr).str(), m_remoteport, socket);
            
			m_socketlist.Add(socket1);
			context1->m_destsocket = socket1;
		}
		else {
            log(server, socket, AString("failed to connect to %s:%u").Arg(_AString(m_remoteaddr)).Arg(m_remoteport));

			wxString msg;
			msg.Printf(_T("Unable to connect to %s:%u"), m_remoteaddr.c_str(), m_remoteport);

			wxMessageDialog dlg(this, msg, _T("Connection Failure"), wxICON_ERROR | wxOK);
			dlg.ShowModal();

			server->DeleteHandler(context2->m_destsocket);
		}
	}
	else {
		wxString msg;
		msg.Printf(_T("Unable to allocate resouces for connection to %s:%u"), m_remoteaddr.c_str(), m_remoteport);

		wxMessageDialog dlg(this, msg, _T("Connection Failure"), wxICON_ERROR | wxOK);
		dlg.ShowModal();

		server->DeleteHandler(socket);

		if (context1) delete context1;
		if (context2) delete context2;
	}
}

void RedirectorWindowFrame::readhandler(ASocketServer *server, int socket, ConnectionContext *context)
{
	static uint8_t buffer[4096];
	int bytes;

	if ((bytes = server->ReadSocket(socket, buffer, sizeof(buffer))) > 0) {
		outputdata(server, socket, "recv", buffer, bytes);

		*context->m_total += bytes;

		if (server->WriteSocket(context->m_destsocket, buffer, bytes) < 0) {
            log(server, context->m_destsocket, AString("failed to write %d bytes, closing connection").Arg(bytes));
			server->DeleteHandler(socket);
		}
		else outputdata(server, context->m_destsocket, "sent", buffer, bytes);
	}
	else {
        log(server, socket, "failed to read data, closing connection");
        server->DeleteHandler(socket);
    }
}

void RedirectorWindowFrame::destructor(ASocketServer *server, int socket, ConnectionContext *context)
{
	if (context) {
		if (context->m_destsocket >= 0) {
            log(server, context->m_destsocket, "closing connection");
			server->DeleteHandler(context->m_destsocket);
			m_socketlist.Remove(context->m_destsocket);
			m_socketlist.Remove(socket);
		}

		delete context;
	}
}

void RedirectorWindowFrame::multi_connectionhandler(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	(void)server;
	m_socketlist.Add(socket);
	context->m_socketlist.Add(socket);
}

void RedirectorWindowFrame::multi_readhandler(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	static uint8_t buffer[4096];
	int bytes;

	if ((bytes = server->ReadSocket(socket, buffer, sizeof(buffer))) > 0) {
		const ADataList& socketlist = context->m_socketlist;
		uint_t i;

		outputdata(server, socket, "recv", buffer, bytes);

		*context->m_total += bytes;

		for (i = 0; i < socketlist.Count(); i++) {
			int socket2 = socketlist[i];

			if (socket2 != socket) {
				if (server->WriteSocket(socket2, buffer, bytes) < 0) {
                    log(server, socket2, AString("failed to write %d bytes, closing connection").Arg(bytes));
					server->DeleteHandler(socket2);
				}
				else outputdata(server, socket2, "sent", buffer, bytes);
			}
		}
	}
	else {
        log(server, socket, "failed to read data, closing connection");
        server->DeleteHandler(socket);
    }
}

void RedirectorWindowFrame::multi_destructor(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	(void)server;
	if (context) {
        log(server, socket, "closing connection");
		m_socketlist.Remove(socket);
		context->m_socketlist.Remove(socket);

		if (context->m_socketlist.Count() == 0) {
			if (context->m_partner && (context->m_partner->m_partner == context)) {
				context->m_partner->m_partner = NULL;
			}
			delete context;
		}
	}
}

void RedirectorWindowFrame::server_connectionhandler(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	(void)server;
	m_socketlist.Add(socket);
	context->m_socketlist.Add(socket);
    log(server, socket, "new connection");
}

AString RedirectorWindowFrame::GetIPAddress(ASocketServer *server, int socket)
{
    AString res;

	res.printf("%s:%u[%d]", server->GetClientAddr(socket).str(), server->GetClientPort(socket), socket);
    
    return res;
}

void RedirectorWindowFrame::log(ASocketServer *server, int socket, const AString& str)
{
    if (m_outputtimestamp) {
        debug("%s: %s: %s\n", ADateTime().DateFormat("%Y-%M-%D %h:%m:%s.%S").str(), GetIPAddress(server, socket).str(), str.str());
    }
    else {
        debug("%s: %s\n", GetIPAddress(server, socket).str(), str.str());
    }        
}

void RedirectorWindowFrame::outputdata(ASocketServer *server, int socket, const char *desc, const uint8_t *data, uint_t bytes)
{
	if (m_outputdebug) {
		// output debug information
		AString str;
		uint_t  i;

		str.printf("%s %u bytes:", desc, bytes);
		for (i = 0; i < bytes; i++) {
			str.printf(" %02x", (uint_t)data[i]);
		}
        str.printf(" : '");
		for (i = 0; i < bytes; i++) {
            if (isprint(data[i])) str.printf("%c", (uint_t)data[i]);
            else                  str.printf(".");
        }
        str.printf("'");
                
        log(server, socket, str);
	}
}

void RedirectorWindowFrame::server_readhandler(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	static uint8_t buffer[4096];
	int bytes;

	if ((bytes = server->ReadSocket(socket, buffer, sizeof(buffer))) > 0) {
		outputdata(server, socket, "recv", buffer, bytes);

		*context->m_total += bytes;

		if (context->m_partner) {
			const ADataList& socketlist = context->m_partner->m_socketlist;
			uint_t i;

			for (i = 0; i < socketlist.Count(); i++) {
				int socket2 = socketlist[i];

				if (server->WriteSocket(socket2, buffer, bytes) < 0) {
                    log(server, socket, AString("failed to write %d bytes, closing connection").Arg(bytes));
					server->DeleteHandler(socket2);
				}
				else outputdata(server, socket2, "sent", buffer, bytes);
			}
		}
		else server->DeleteHandler(socket);
	}
	else {
        log(server, socket, "failed to read data, closing connection");
        server->DeleteHandler(socket);
    }
}

void RedirectorWindowFrame::server_destructor(ASocketServer *server, int socket, MultiConnectionContext *context)
{
	(void)server;
	if (context) {
        log(server, socket, "closing connection");

		m_socketlist.Remove(socket);
		context->m_socketlist.Remove(socket);
#if 0
		if (context->m_socketlist.Count() == 0) {
			if (context->m_partner && (context->m_partner->m_partner == context)) {
				context->m_partner->m_partner = NULL;
			}
			delete context;
		}
#endif
	}
}

void RedirectorWindowFrame::OnTimer(wxTimerEvent &)
{
	Refresh(false);
}

void RedirectorWindowFrame::OnPaint(wxPaintEvent &)
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

	bool rx_changed = (m_rxbytes != m_last_rxbytes);
	bool tx_changed = (m_txbytes != m_last_txbytes);

	m_last_rxbytes = m_rxbytes;
	m_last_txbytes = m_txbytes;

	uint_t cnt = m_socketlist.Count();
	uint_t rxk = (uint_t)((m_last_rxbytes + 1023) / 1024);
	uint_t txk = (uint_t)((m_last_txbytes + 1023) / 1024);

	if (cnt == 1) {
		str.Printf(_T("%u connection"), cnt);
	}
	else {
		str.Printf(_T("%u connections"), cnt);
	}
	dc.SetTextForeground(wxColour(0, 0, 0));
	dc.DrawText(str, pos.x, pos.y); pos.y += sz_h + 10;

	if (rx_changed) {
		wxBrush br(wxColour(255, 0, 0));
		wxPen pen(wxColour(128, 0, 0));

		dc.SetBrush(br);
		dc.SetPen(pen);
		dc.DrawRectangle(5, pos.y, sz_h, sz_h);
		dc.SetPen(wxNullPen);
		dc.SetBrush(wxNullBrush);
	}
	str.Printf(_T("Rx: %uk"), rxk);
	dc.SetTextForeground(wxColour(255, 0, 0));
	dc.DrawText(str, pos.x, pos.y); pos.y += sz_h + 10;

	if (tx_changed) {
		wxBrush br(wxColour(0, 0, 255));
		wxPen pen(wxColour(0, 0, 128));

		dc.SetBrush(br);
		dc.SetPen(pen);
		dc.DrawRectangle(5, pos.y, sz_h, sz_h);
		dc.SetPen(wxNullPen);
		dc.SetBrush(wxNullBrush);
	}
	str.Printf(_T("Tx: %uk"), txk);
	dc.SetTextForeground(wxColour(0, 0, 255));
	dc.DrawText(str, pos.x, pos.y); pos.y += sz_h + 10;
	
	dc.SetFont(wxNullFont);
}
