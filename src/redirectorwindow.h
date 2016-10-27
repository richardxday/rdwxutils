
#ifndef __REDIRECTOR_WINDOW__
#define __REDIRECTOR_WINDOW__

#include <rdlib/wxsup.h>

#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <rdlib/SocketServer.h>

class RedirectorWindowFrame: public wxFrame
{
public:
    RedirectorWindowFrame(wxWindow *parent,
						  const wxString& title, const wxPoint& pos, const wxSize& size,
						  ASocketServer& server, uint_t type,
						  const wxString& localaddr, uint_t localport,
						  const wxString& remoteaddr, uint_t remoteport,
						  void (*closecallback)(wxFrame *frame, void *context), void *closecontext);
    virtual ~RedirectorWindowFrame();

	enum {
		Type_Server_Client = 0,			// server - client mode: create a local server forwarding to a remote server as a client
		Type_Client_Client,				// client - client mode: connect to a local server as a client and forward to a remote server as a client
		Type_Server_Server,				// server - server mode: create local and remote servers and forward data between sets of clients
		Type_Multi_Client,				// multi client mode:    multiple clients connect to a single server
	};

	bool Valid() const {return (m_socketlist.Count() > 0);}

    void OnToggleDebug(wxCommandEvent &);
    void OnToggleTimestamp(wxCommandEvent &);
    void OnClose(wxCommandEvent &);
	void OnTimer(wxTimerEvent &);
    void OnPaint(wxPaintEvent & event);

    DECLARE_EVENT_TABLE()

protected:
    AString GetIPAddress(ASocketServer *server, int socket);
    void log(ASocketServer *server, int socket, const AString& str);
	void outputdata(ASocketServer *server, int socket, const char *desc, const uint8_t *data, uint_t bytes);

	typedef struct {
		RedirectorWindowFrame *m_frame;
		int      m_destsocket;				// destination socket
		uint64_t *m_total;                  // ptr to data accumulator
	} ConnectionContext;

	typedef struct _MultiConnectionContext {
		RedirectorWindowFrame *m_frame;
		uint64_t  *m_total;                 // ptr to data accumulator
		ADataList m_socketlist;
		struct _MultiConnectionContext *m_partner;
	} MultiConnectionContext;

	// static callbacks -> all redirect back into appropriate frame class
	static void __connectionhandler(ASocketServer *server, int socket, void *context) {
		((ConnectionContext *)context)->m_frame->connectionhandler(server, socket, (ConnectionContext *)context);
	}
	static void __readhandler(ASocketServer *server, int socket, void *context) {
		((ConnectionContext *)context)->m_frame->readhandler(server, socket, (ConnectionContext *)context);
	}
	static void __destructor(ASocketServer *server, int socket, void *context) {
		((ConnectionContext *)context)->m_frame->destructor(server, socket, (ConnectionContext *)context);
	}

	static void __multi_connectionhandler(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->multi_connectionhandler(server, socket, (MultiConnectionContext *)context);
	}
	static void __multi_readhandler(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->multi_readhandler(server, socket, (MultiConnectionContext *)context);
	}
	static void __multi_destructor(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->multi_destructor(server, socket, (MultiConnectionContext *)context);
	}

	static void __server_connectionhandler(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->server_connectionhandler(server, socket, (MultiConnectionContext *)context);
	}
	static void __server_readhandler(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->server_readhandler(server, socket, (MultiConnectionContext *)context);
	}
	static void __server_destructor(ASocketServer *server, int socket, void *context) {
		((MultiConnectionContext *)context)->m_frame->server_destructor(server, socket, (MultiConnectionContext *)context);
	}

	void connectionhandler(ASocketServer *server, int socket, ConnectionContext *context);
	void readhandler(ASocketServer *server, int socket, ConnectionContext *context);
	void destructor(ASocketServer *server, int socket, ConnectionContext *context);

	void multi_connectionhandler(ASocketServer *server, int socket, MultiConnectionContext *context);
	void multi_readhandler(ASocketServer *server, int socket, MultiConnectionContext *context);
	void multi_destructor(ASocketServer *server, int socket, MultiConnectionContext *context);

	void server_connectionhandler(ASocketServer *server, int socket, MultiConnectionContext *context);
	void server_readhandler(ASocketServer *server, int socket, MultiConnectionContext *context);
	void server_destructor(ASocketServer *server, int socket, MultiConnectionContext *context);

private:
	ASocketServer& m_server;
	ADataList m_socketlist;
	wxMenu   *m_menu;
	wxTimer  m_timer;
	wxFont   m_font;
	wxString m_remoteaddr;
	uint_t   m_remoteport;
	void 	 (*m_closecallback)(wxFrame *frame, void *context);
	void 	 *m_closecontext;
	uint64_t m_rxbytes, m_txbytes;
	uint64_t m_last_rxbytes, m_last_txbytes;
	bool     m_outputdebug;
	bool     m_outputtimestamp;
};

#endif
