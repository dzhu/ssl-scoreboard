#pragma once

#include <wx/wx.h>

#include "messages_robocup_ssl_geometry.pb.h"
#include "ssl_autoref.pb.h"

#include "shared/world.h"

class ScoreboardApp;

class FieldPanel : public wxPanel
{
  wxSizer *sizer;
  ScoreboardApp *board;

  SSL_GeometryData geo;

  void render(wxPaintEvent &event);
  void drawRobot(wxGraphicsContext &gc, WorldRobot r);

public:
  FieldPanel(wxWindow *parent, wxWindowID id, ScoreboardApp *board);
};
