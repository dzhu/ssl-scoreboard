#include <wx/wx.h>
#include <deque>

#include "ssl_autoref.pb.h"

class ScoreboardApp;

class HistoryPanel : public wxPanel
{
  unsigned int max_items;

public:
  HistoryPanel() : wxPanel(), max_items(0)
  {
  }

  HistoryPanel(wxWindow *parent, wxWindowID id, int max_items) : wxPanel(parent, id), max_items(max_items)
  {
    SetSizer(new wxBoxSizer(wxVERTICAL));
  }

  void update(const ssl::SSL_Autoref &a);
};
