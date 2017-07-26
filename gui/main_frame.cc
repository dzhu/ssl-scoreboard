#include "main_frame.h"

#include "scoreboard.h"

#include <wx/gbsizer.h>
#include "gui_utils.h"

using gbp = wxGBPosition;

void ScoreboardFrame::OnClose(wxCloseEvent &event)
{
  board->Exit();
  event.Skip();
}

ScoreboardFrame::ScoreboardFrame(const wxString &title, ScoreboardApp *board)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1400, 600)), board(board)
{
  Bind(wxEVT_CLOSE_WINDOW, &ScoreboardFrame::OnClose, this);

  wxIcon icon(_T("robocup.png"), wxBITMAP_TYPE_PNG);
  SetIcon(icon);

  auto sizer = new wxGridBagSizer(10, 10);
  SetSizer(sizer);

  sizer->Add(15, 15, gbp(0, 0));
  sizer->Add(15, 15, gbp(5, 2));
  sizer->Add(15, 15, gbp(0, 4));
  sizer->AddGrowableRow(4);
  sizer->AddGrowableCol(1, 3);
  sizer->AddGrowableCol(3, 7);

  wxFont top_label_font(32, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);

  // wxStaticText *info_label = makeText(this, "Game information", top_label_font);
  wxStaticText *history_label = makeText(this, "Referee decisions", top_label_font);

  // sizer->Add(info_label, gbp(1, 1), wxDefaultSpan, wxALIGN_LEFT);
  sizer->Add(history_label, gbp(1, 3), wxDefaultSpan, wxALIGN_LEFT);

  info_panel = new GameInfoPanel(this, wxID_ANY, board);
  sizer->Add(info_panel, gbp(1, 1), wxGBSpan(2, 1), wxEXPAND);

  history_panel = new HistoryPanel(this, wxID_ANY, 5);
  sizer->Add(history_panel, gbp(2, 3), wxGBSpan(3, 1), wxEXPAND);

  field_panel = new FieldPanel(this, wxID_ANY, board);
  sizer->Add(field_panel, gbp(4, 1), wxDefaultSpan, wxEXPAND);

  sizer->Add(10, 10, gbp(3, 1));

  history_label->SetMinSize(wxSize(0, 0));
  history_panel->SetMinSize(wxSize(0, 0));
  info_panel->SetMinSize(wxSize(0, info_panel->GetMinHeight()));

  sizer->Layout();
}
