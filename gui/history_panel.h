#pragma once

#include <wx/wx.h>
#include <deque>

#include "ssl_autoref.pb.h"

class ScoreboardApp;

class HistoryPanel : public wxPanel
{
  unsigned int max_items;
  ScoreboardApp *board;

public:
  HistoryPanel() : wxPanel(), max_items(0)
  {
  }

  HistoryPanel(wxWindow *parent, wxWindowID id, ScoreboardApp *board, int max_items)
      : wxPanel(parent, id), board(board), max_items(max_items)
  {
    SetSizer(new wxBoxSizer(wxVERTICAL));
  }

  void update(const ssl::SSL_Autoref &a);
};
