#include "info_panel.h"

#include <wx/gbsizer.h>
using gbp = wxGBPosition;

#include "ssl_referee.pb.h"

#include "scoreboard.h"

#include "gui_utils.h"
#include "shared/util.h"
#include "style.h"

GameInfoPanel::GameInfoPanel(wxWindow *parent, wxWindowID id, ScoreboardApp *board) : wxPanel(parent, id), board(board)
{
  auto sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(sizer);

  wxFont label_font(wxFontInfo(30).Family(wxFONTFAMILY_DEFAULT).Italic());
  wxFont text_font(wxFontInfo(30).Family(wxFONTFAMILY_DEFAULT));
  wxFont name_font(wxFontInfo(45).Family(wxFONTFAMILY_DEFAULT));
  wxFont score_font(wxFontInfo(65).Family(wxFONTFAMILY_DEFAULT));

  wxFont time_label_font(wxFontInfo(50).Family(wxFONTFAMILY_DEFAULT).Italic());
  wxFont time_text_font(wxFontInfo(50).Family(wxFONTFAMILY_DEFAULT));

  {
    auto score_sizer = new wxGridBagSizer(0, 0);

    blue_name_text = makeText(this, "Blue", name_font, blue_team_colour);
    blue_score_text = makeText(this, "0", score_font);
    yellow_name_text = makeText(this, "Yellow", name_font, yellow_team_colour);
    yellow_score_text = makeText(this, "0", score_font);

    // blue_name_text->SetMinSize(wxSize(20, blue_name_text->GetMinHeight()));
    // blue_score_text->SetMinSize(wxSize(20, blue_score_text->GetMinHeight()));
    // yellow_name_text->SetMinSize(wxSize(20, yellow_name_text->GetMinHeight()));
    // yellow_score_text->SetMinSize(wxSize(20, yellow_score_text->GetMinHeight()));

    score_sizer->Add(blue_name_text, gbp(0, 1), wxDefaultSpan, wxALIGN_CENTER_HORIZONTAL);
    score_sizer->Add(yellow_name_text, gbp(0, 3), wxDefaultSpan, wxALIGN_CENTER_HORIZONTAL);
    score_sizer->Add(blue_score_text, gbp(1, 1), wxDefaultSpan, wxALIGN_CENTER_HORIZONTAL);
    score_sizer->Add(yellow_score_text, gbp(1, 3), wxDefaultSpan, wxALIGN_CENTER_HORIZONTAL);

    score_sizer->Add(0, 0, gbp(0, 0));
    score_sizer->Add(0, 0, gbp(0, 2));
    score_sizer->Add(0, 0, gbp(0, 4));

    score_sizer->AddGrowableCol(0, 2);
    score_sizer->AddGrowableCol(1, 3);
    score_sizer->AddGrowableCol(2, 1);
    score_sizer->AddGrowableCol(3, 3);
    score_sizer->AddGrowableCol(4, 2);

    sizer->Add(score_sizer, 0, wxEXPAND);
  }

  {
    auto ref_sizer = new wxGridBagSizer(0, 0);

    wxStaticText *stage_label = makeText(this, "Stage:", label_font, wxColour(0, 0, 0));
    wxStaticText *time_label = makeText(this, "Time left:", time_label_font, wxColour(0, 0, 0));
    wxStaticText *command_label = makeText(this, "Command:", label_font, wxColour(0, 0, 0));

    stage_text = makeText(this, "", text_font);
    time_text = makeText(this, "", time_text_font);
    command_text = makeText(this, "", text_font);

    ref_sizer->Add(time_label, gbp(0, 0), wxDefaultSpan);
    ref_sizer->Add(command_label, gbp(1, 0), wxDefaultSpan);
    ref_sizer->Add(stage_label, gbp(2, 0), wxDefaultSpan);

    ref_sizer->Add(time_text, gbp(0, 2), wxDefaultSpan);
    ref_sizer->Add(command_text, gbp(1, 2), wxDefaultSpan);
    ref_sizer->Add(stage_text, gbp(2, 2), wxDefaultSpan);

    ref_sizer->Add(20, 0, gbp(0, 1));

    sizer->Add(ref_sizer, 0, wxALIGN_LEFT);
  }
}

void GameInfoPanel::update()
{
  const SSL_Referee &msg = board->referee_msg;

  std::string blue_name = msg.blue().name();
  std::string yellow_name = msg.yellow().name();

  if (!blue_name.size()) {
    blue_name = "Blue";
  }
  if (!yellow_name.size()) {
    yellow_name = "Yellow";
  }

  blue_name_text->SetLabel(ws(blue_name));
  yellow_name_text->SetLabel(ws(yellow_name));

  blue_score_text->SetLabel(wxString::Format(_T("%d"), msg.blue().score()));
  yellow_score_text->SetLabel(wxString::Format(_T("%d"), msg.yellow().score()));

  stage_text->SetLabel(ws(stageDisplayName(msg.stage())));
  command_text->SetLabel(ws(commandDisplayName(msg.command())));
  time_text->SetLabel(formatTime(msg.stage_time_left()));

  if (msg.stage_time_left() < 0) {
    time_text->SetForegroundColour(wxColour(255, 64, 64));
  }
  else {
    time_text->SetBackgroundColour(wxNullColour);
  }

  GetSizer()->Layout();
}
