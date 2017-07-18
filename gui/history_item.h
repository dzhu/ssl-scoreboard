#pragma once

#include <wx/gbsizer.h>
#include <wx/wx.h>

#include "ssl_autoref.pb.h"
#include "ssl_referee.pb.h"

class wxRefereeHistoryItem : public wxPanel
{
  wxSizer *sizer;
  bool emph;
  wxColour start_colour;

  wxStaticText *call_label, *call_text, *next_label, *next_text;
  wxStaticText *description_label, *description_text, *stage_time_label, *stage_time_text;
  ~wxRefereeHistoryItem();

  wxTimer *animate_timer;

  uint64_t create_time;

public:
  wxRefereeHistoryItem(wxWindow *parent, wxWindowID id, ssl::SSL_Autoref update);

  void setEmph(bool e);

  void OnTimer(wxTimerEvent &event);
};
