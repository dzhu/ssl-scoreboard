#include <wx/gbsizer.h>

#include "control_frame.h"
#include "event_types.h"
#include "gui_utils.h"
#include "scoreboard.h"

using gbp = wxGBPosition;

void ScoreboardControlFrame::OnClose(wxCloseEvent &event)
{
  board->Exit();
  event.Skip();
}

ScoreboardControlFrame::ScoreboardControlFrame(const wxString &title, ScoreboardApp *board)
    : wxFrame(nullptr, wxID_ANY, title), board(board)
{
  Bind(wxEVT_CLOSE_WINDOW, &ScoreboardControlFrame::OnClose, this);

  wxIcon icon(_T("robocup.png"), wxBITMAP_TYPE_PNG);
  SetIcon(icon);

  auto sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(sizer);

  wxCheckBox *enable_replays = new wxCheckBox(this, wxID_ANY, "Enable replays");
  enable_replays->SetValue(true);
  sizer->Add(enable_replays, 0);
  enable_replays->Bind(wxEVT_CHECKBOX,
                       [=](wxCommandEvent &event) { board->setEnableReplays(enable_replays->GetValue()); });

  // wxCheckBox *follow_ball = new wxCheckBox(this, wxID_ANY, "Replays follow ball");
  // follow_ball->SetValue(true);
  // sizer->Add(follow_ball, 0);
  // follow_ball->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent &event) { board->setFollowBall(follow_ball->GetValue()); });
}
