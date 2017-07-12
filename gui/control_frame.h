#pragma once

#include <vector>

#include <wx/wx.h>

class ScoreboardApp;

class ScoreboardControlFrame : public wxFrame
{
  ScoreboardApp *board;

  void OnQuit(wxCommandEvent &event)
  {
    Close(true);
  }
  void OnClose(wxCloseEvent &event);

public:
  ScoreboardControlFrame(const wxString &title, ScoreboardApp *board);
};
