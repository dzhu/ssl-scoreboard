#include <vector>

#include <wx/wx.h>

class ScoreboardApp;

class ScoreboardControlFrame : public wxFrame
{
  ScoreboardApp *board;

public:
  ScoreboardControlFrame(const wxString &title, ScoreboardApp *board);

private:
  void OnQuit(wxCommandEvent &event)
  {
    Close(true);
  }
};
