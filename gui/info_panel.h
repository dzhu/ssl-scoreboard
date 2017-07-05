#pragma once

#include <wx/wx.h>

class ScoreboardApp;

class GameInfoPanel : public wxPanel
{
  ScoreboardApp *board;
  wxStaticText *blue_name_text, *yellow_name_text;
  wxStaticText *blue_score_text, *yellow_score_text;
  wxStaticText *stage_text, *time_text, *command_text;

public:
  GameInfoPanel(wxWindow *parent, wxWindowID id, ScoreboardApp *board);
  void update();
};
