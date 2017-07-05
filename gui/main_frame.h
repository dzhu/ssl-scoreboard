#include <wx/wx.h>
#include "history_item.h"
#include "info_panel.h"
#include "history_panel.h"
#include "field_panel.h"

class ScoreboardApp;

class ScoreboardFrame : public wxFrame
{
  ScoreboardApp *board;

  // void OnQuit(wxCommandEvent &event)
  // {
  //   puts("q");
  //   Close(true);
  // }

  GameInfoPanel *info_panel;
  HistoryPanel *history_panel;
  FieldPanel *field_panel;

  friend class ScoreboardApp;

public:
  ScoreboardFrame(const wxString &title, ScoreboardApp *board);
};
