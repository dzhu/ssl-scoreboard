#include <wx/gbsizer.h>

#include "control_frame.h"
#include "event_types.h"
#include "gui_utils.h"

using gbp = wxGBPosition;

ScoreboardControlFrame::ScoreboardControlFrame(const wxString &title, ScoreboardApp *board)
    : wxFrame(nullptr, wxID_ANY, title), board(board)
{
  wxIcon icon(_T("robocup.png"), wxBITMAP_TYPE_PNG);
  SetIcon(icon);

  wxGridBagSizer *full_sizer = new wxGridBagSizer(0, 0);
  SetSizer(full_sizer);
  full_sizer->Add(20, 20, gbp(0, 0));
  full_sizer->Add(20, 20, gbp(2, 2));
  full_sizer->AddGrowableRow(1, 1);
  full_sizer->AddGrowableCol(1, 1);
  wxCheckBox *c = new wxCheckBox(this, wxID_ANY, "h");
  full_sizer->Add(c, gbp(1, 1), wxDefaultSpan, wxEXPAND);
  printf("%p\n", this->board);
}
