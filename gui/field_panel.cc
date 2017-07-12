#include "field_panel.h"

#include <wx/graphics.h>
#include "scoreboard.h"

#include "gui_utils.h"
#include "shared/util.h"
#include "style.h"

FieldPanel::FieldPanel(wxWindow *parent, wxWindowID id, ScoreboardApp *board)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      sizer(new wxBoxSizer(wxVERTICAL)),
      board(board)
{
  Bind(wxEVT_PAINT, &FieldPanel::render, this, wxID_ANY);
}

void FieldPanel::drawRobot(wxGraphicsContext &gc, WorldRobot r)
{
  static const double R = 90;
  static const int N = 20;
  static const double th0 = acos(55. / 85.);
  static const double th1 = 2 * M_PI - th0;

  double verts[N][2];
  for (int i = 0; i < N; i++) {
    double th = r.angle + th0 + (th1 - th0) * i / (N - 1);
    verts[i][0] = r.loc.x + R * cos(th);
    verts[i][1] = r.loc.y + R * sin(th);
  }

  gc.SetAntialiasMode(wxANTIALIAS_DEFAULT);
  gc.SetPen(wxPen(wxColour(0, 0, 0), 15));
  gc.SetBrush(wxBrush(r.team == TeamBlue ? blue_team_colour : yellow_team_colour));

  if (0) {
    wxGraphicsPath path = gc.CreatePath();
    path.AddLineToPoint(0, 0);
    path.AddLineToPoint(500, 0);
    path.AddLineToPoint(500, 500);
    path.AddLineToPoint(0, 500);
    path.CloseSubpath();
    gc.FillPath(path);
  }

  wxGraphicsPath path = gc.CreatePath();
  for (int i = 0; i < N; i++) {
    path.AddLineToPoint(verts[i][0], verts[i][1]);
  }
  path.CloseSubpath();
  gc.FillPath(path);
  gc.StrokePath(path);
}

void FieldPanel::render(wxPaintEvent &event)
{
  // TODO we assume our local machine time matches closely with autoref and
  // vision message times
  uint64_t now = GetTimeMicros();
  uint64_t replay_time = board->replay_start + now - board->replay_actual_start;
  bool is_replay = replay_time < board->replay_end;

  wxPaintDC dc(this);
  wxGraphicsContext *gc_p = wxGraphicsContext::Create(dc);

  if (gc_p == nullptr) {
    puts("couldn't make drawing context!");
    return;
  }

  wxGraphicsContext &gc = *gc_p;

  gc.SetAntialiasMode(wxANTIALIAS_DEFAULT);

  SSL_GeometryData &geo = board->geometry_msg;
  if (!geo.has_field()) {
    puts("no geo!");

    SSL_GeometryFieldSize &dims = *geo.mutable_field();
    dims.set_field_length(9000);
    dims.set_field_width(6000);
    dims.set_goal_width(1000);
    dims.set_goal_depth(200);
    dims.set_defense_radius(1000);
    dims.set_defense_stretch(500);
    dims.set_center_circle_radius(500);
  }

  const SSL_GeometryFieldSize &dims = geo.field();
  wxSize sz = GetSize();

  double fwh = dims.field_width() / 2, flh = dims.field_length() / 2;
  double gd = dims.goal_depth(), gwh = dims.goal_width() / 2;
  double dr = dims.defense_radius(), dsh = dims.defense_stretch() / 2;
  double ccr = dims.center_circle_radius();

  double margin = gd + 80;
  double min_x = -flh - margin;
  double max_x = flh + margin;
  double min_y = -fwh - margin;
  double max_y = fwh + margin;

  double scale = std::min(sz.y / (max_y - min_y), sz.x / (max_x - min_x));
  gc.Translate(sz.x / 2, sz.y / 2);
  gc.Scale(scale, -scale);

  // draw background green rectangle
  {
    wxGraphicsPath path = gc.CreatePath();
    path.AddRectangle(min_x, min_y, max_x - min_x, max_y - min_y);

    gc.SetBrush(wxBrush(is_replay ? wxColour(0, 65, 0) : wxColour(0, 128, 0)));
    gc.FillPath(path);
  }

  // // fill goals
  // int blue_side = vars.blue_side;
  // if (blue_side != 0) {
  //   dc.SetPen(wxNullPen);

  //   dc.SetBrush(wxBrush(blue_side > 0 ? blue_team_colour : yellow_team_colour));
  //   dc.DrawRectangle(flh, -gwh, gd, gwh * 2);

  //   dc.SetBrush(wxBrush(blue_side > 0 ? yellow_team_colour : blue_team_colour));
  //   dc.DrawRectangle(-flh, gwh, -gd, -gwh * 2);
  // }

  // draw field lines
  {
    wxGraphicsPath path = gc.CreatePath();
    path.MoveToPoint(0, -fwh);
    path.AddLineToPoint(0, fwh);
    path.AddRectangle(-flh, -fwh, flh * 2, fwh * 2);

    path.AddCircle(0, 0, ccr);
    path.MoveToPoint(-flh, dsh + dr);
    path.AddArc(-flh, dsh, dr, M_PI_2, 0, false);
    path.AddLineToPoint(-flh + dr, -dsh);
    path.AddArc(-flh, -dsh, dr, 0, -M_PI_2, false);

    path.MoveToPoint(flh, -dsh - dr);
    path.AddArc(flh, -dsh, dr, -M_PI_2, -M_PI, false);
    path.AddLineToPoint(flh - dr, dsh);
    path.AddArc(flh, dsh, dr, -M_PI, M_PI_2, false);

    path.MoveToPoint(-flh, gwh);
    path.AddLineToPoint(-flh - gd, gwh);
    path.AddLineToPoint(-flh - gd, -gwh);
    path.AddLineToPoint(-flh, -gwh);

    path.MoveToPoint(flh, -gwh);
    path.AddLineToPoint(flh + gd, -gwh);
    path.AddLineToPoint(flh + gd, gwh);
    path.AddLineToPoint(flh, gwh);

    gc.SetPen(wxPen(wxColour(255, 255, 255), 20));
    gc.StrokePath(path);
  }

  // draw world objects
  World *world = nullptr;
  if (is_replay) {
    for (auto &w : board->world_history) {
      if (w.time * 1e6 < replay_time) {
        world = &w;
      }
      else {
        break;
      }
    }
  }
  else {
    world = &board->world;
  }

  if (world != nullptr) {
    // draw robots
    for (auto r : world->robots) {
      drawRobot(gc, r);
    }

    // draw all detected balls
    {
      wxGraphicsPath path = gc.CreatePath();
      path.AddCircle(world->ball.loc.x, world->ball.loc.y, 40);

      gc.SetBrush(wxBrush(wxColour(255, 128, 0)));
      gc.FillPath(path);

      gc.SetPen(wxPen(wxColour(0, 0, 0), 15));
      gc.StrokePath(path);
    }
  }

  delete gc_p;

  Update();
}
