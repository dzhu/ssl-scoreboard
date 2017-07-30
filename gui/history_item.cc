#include "gui_utils.h"
#include "shared/util.h"
#include "style.h"

#include "history_item.h"

#include "scoreboard.h"

using gbp = wxGBPosition;

std::string outDescription(const ssl::SSL_Autoref &update)
{
  char buf[200];

  auto out_point = update.ball_out_of_field().position();
  bool past_goal_line = fabs(out_point.x()) - fabs(out_point.y()) > FieldLengthH - FieldWidthH;
  if (past_goal_line) {
    bool own_half
      = (update.ball_out_of_field().last_touch() == ssl::SSL_Autoref::BLUE) == (update.blue_side() * out_point.x() > 0);
    sprintf(buf,
            own_half ? "Ball out (corner kick, %.0f,%.0f)" : "Ball out (goal kick, %.0f,%.0f)",
            out_point.x(),
            out_point.y());
    return std::string(buf);
  }
  else {
    sprintf(buf, "Ball out (throw in, %.0f,%.0f)", out_point.x(), out_point.y());
    return std::string(buf);
  }
}

std::string infringementDescription(ssl::SSL_Autoref a)
{
  switch (a.foul().foul_type()) {
    case ssl::SSL_Autoref::RuleInfringement::BALL_SPEED:
      return "ball speed";
    case ssl::SSL_Autoref::RuleInfringement::DEFENDER_DEFENSE_AREA_PARTIAL:
      return "defense area (partial)";
    case ssl::SSL_Autoref::RuleInfringement::DEFENDER_DEFENSE_AREA_FULL:
      return "defense area (full)";
    case ssl::SSL_Autoref::RuleInfringement::ATTACKER_DEFENSE_AREA:
      return "opponent defense area";
    case ssl::SSL_Autoref::RuleInfringement::DEFENSE_AREA_DISTANCE:
      return "defense area margin";
    case ssl::SSL_Autoref::RuleInfringement::COLLISION:
      return "collision";
    case ssl::SSL_Autoref::RuleInfringement::DOUBLE_TOUCH:
      return "double touch";
    case ssl::SSL_Autoref::RuleInfringement::DRIBBLING:
      return "dribble distance";
    case ssl::SSL_Autoref::RuleInfringement::STOP_BALL_DISTANCE:
      return "STOP ball distance";
    case ssl::SSL_Autoref::RuleInfringement::STOP_SPEED:
      return "STOP robot speed";
    case ssl::SSL_Autoref::RuleInfringement::NUMBER_OF_PLAYERS:
      return "too many players";
    case ssl::SSL_Autoref::RuleInfringement::CARPETING:
      return "carpeting";
    case ssl::SSL_Autoref::RuleInfringement::OUT_OF_FIELD:
      return "out of field";
    case ssl::SSL_Autoref::RuleInfringement::CHIP_GOAL:
      return "chip goal";
  }
}

SSL_Referee::Command getNextCommand(ssl::SSL_Autoref a)
{
  switch (a.event_case()) {
    case ssl::SSL_Autoref::kBallOutOfField: {
      Team team;
      TeamCommand c;

      switch (a.ball_out_of_field().last_touch()) {
        case ssl::SSL_Autoref::BLUE:
          team = TeamYellow;
          break;
        case ssl::SSL_Autoref::YELLOW:
          team = TeamBlue;
          break;
      }

      auto out_point = a.ball_out_of_field().position();
      bool past_goal_line = fabs(out_point.x()) - fabs(out_point.y()) > FieldLengthH - FieldWidthH;
      if (past_goal_line) {
        c = DIRECT_FREE;
      }
      else {
        c = INDIRECT_FREE;
      }

      return teamCommand(c, team);
    }

    case ssl::SSL_Autoref::kFoul: {
      Team team;
      TeamCommand c;

      switch (a.foul().offending_team()) {
        case ssl::SSL_Autoref::BLUE:
          team = TeamYellow;
          break;
        case ssl::SSL_Autoref::YELLOW:
          team = TeamBlue;
          break;
      }

      switch (a.foul().foul_type()) {
        case ssl::SSL_Autoref::RuleInfringement::BALL_SPEED:
        case ssl::SSL_Autoref::RuleInfringement::ATTACKER_DEFENSE_AREA:
        case ssl::SSL_Autoref::RuleInfringement::DEFENSE_AREA_DISTANCE:
        case ssl::SSL_Autoref::RuleInfringement::DOUBLE_TOUCH:
        case ssl::SSL_Autoref::RuleInfringement::DRIBBLING:
        case ssl::SSL_Autoref::RuleInfringement::CARPETING:
        case ssl::SSL_Autoref::RuleInfringement::CHIP_GOAL:
          c = INDIRECT_FREE;
          break;

        case ssl::SSL_Autoref::RuleInfringement::COLLISION:
          c = DIRECT_FREE;
          break;

        case ssl::SSL_Autoref::RuleInfringement::DEFENDER_DEFENSE_AREA_FULL:
          c = PREPARE_PENALTY;
          break;

        case ssl::SSL_Autoref::RuleInfringement::DEFENDER_DEFENSE_AREA_PARTIAL:
        case ssl::SSL_Autoref::RuleInfringement::STOP_BALL_DISTANCE:
        case ssl::SSL_Autoref::RuleInfringement::STOP_SPEED:
        case ssl::SSL_Autoref::RuleInfringement::NUMBER_OF_PLAYERS:
        case ssl::SSL_Autoref::RuleInfringement::OUT_OF_FIELD:
          // TODO handle things more correctly (work cards in somehow?)
          return SSL_Referee::FORCE_START;
      }

      return teamCommand(c, team);
    }

    case ssl::SSL_Autoref::kLackOfProgress: {
      return SSL_Referee::FORCE_START;
    }

    case ssl::SSL_Autoref::kGoal: {
      switch (a.goal().scoring_team()) {
        case ssl::SSL_Autoref::BLUE:
          return SSL_Referee::PREPARE_KICKOFF_YELLOW;
        case ssl::SSL_Autoref::YELLOW:
          return SSL_Referee::PREPARE_KICKOFF_BLUE;
      }
    }

    case ssl::SSL_Autoref::EVENT_NOT_SET: {
      return SSL_Referee::FORCE_START;
    }
  }
}

wxRefereeHistoryItem::wxRefereeHistoryItem(wxWindow *parent,
                                           wxWindowID id,
                                           ScoreboardApp *board,
                                           const std::vector<ssl::SSL_Autoref> &updates)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED),
      emph(true),
      call_label(nullptr),
      call_text(nullptr),
      next_label(nullptr),
      next_text(nullptr),
      description_label(nullptr),
      description_text(nullptr),
      stage_time_label(nullptr),
      stage_time_text(nullptr),
      board(board)
{
  create_time = GetTimeMicros();

  auto sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(sizer);

  sizer->Add(100, 0);

  call_label = makeText(this, "Call");
  sizer->Add(call_label);

  std::string call_str, robots_str;
  wxColour call_colour;

  ssl::SSL_Autoref update = updates[0];

  for (const auto &u : updates) {
    if (std::abs(update.ball_out_of_field().position().x()) > FieldLengthH - 200
        || std::abs(update.ball_out_of_field().position().y()) > FieldWidthH - 200) {
      update.CopyFrom(u);
      break;
    }
  }

  // default start colour (reddish, to evoke fouls)
  start_colour.Set(255, 128, 128);

  switch (update.event_case()) {
    case ssl::SSL_Autoref::kBallOutOfField: {
      bool isBlue = update.ball_out_of_field().last_touch() == ssl::SSL_Autoref_Team_BLUE;
      call_str = outDescription(update);
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;
    } break;
    case ssl::SSL_Autoref::kFoul: {
      bool isBlue = update.foul().offending_team() == ssl::SSL_Autoref_Team_BLUE;
      call_str = "Foul: " + infringementDescription(update);
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;
    } break;
    case ssl::SSL_Autoref::kGoal: {
      bool isBlue = update.goal().scoring_team() == ssl::SSL_Autoref_Team_BLUE;
      call_str = "Goal!";
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;

      if (update.goal().has_scoring_robot()) {
      }

      // set start colour to green instead
      start_colour.Set(32, 255, 32);
    } break;
    case ssl::SSL_Autoref::kLackOfProgress: {
      if (update.lack_of_progress()) {
        call_str = "Lack of progress";
      }
      break;
    }
    case ssl::SSL_Autoref::EVENT_NOT_SET:
      break;
  }

  SSL_Referee::Command c = getNextCommand(update);

  wxButton *apply_button = new wxButton(this, wxID_ANY, std::string("Send command ") + SSL_Referee::Command_Name(c));
  apply_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) { board->sendCommand(c); });

  wxButton *place_button = new wxButton(this, wxID_ANY, std::string("Send ball placement command"));
  place_button->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
    if (!board->rcon.isOpened()) {
      board->rcon.open("localhost", 10007);
    }

    if (board->rcon.isOpened()) {
      // TODO actually decide on the right command
      board->rcon.sendCommand(SSL_Referee::STOP);

      if (board->comparer.hasDesignatedPoint() && (board->enable_blue_placement || board->enable_yellow_placement)) {
        Team placing_team = TeamNone;

        if (board->enable_blue_placement && !board->enable_yellow_placement) {
          placing_team = TeamBlue;
        }
        else if (!board->enable_blue_placement && board->enable_yellow_placement) {
          placing_team = TeamYellow;
        }
        else {
          auto flip_team
            = [](ssl::SSL_Autoref::Team team) { return team == ssl::SSL_Autoref::BLUE ? TeamYellow : TeamBlue; };

          switch (update.event_case()) {
            case ssl::SSL_Autoref::kBallOutOfField: {
              placing_team = flip_team(update.ball_out_of_field().last_touch());
              break;
            }
            case ssl::SSL_Autoref::kFoul: {
              placing_team = flip_team(update.foul().offending_team());
              break;
            }
            case ssl::SSL_Autoref::kLackOfProgress: {
              placing_team = RandomTeam();
              break;
            }
            case ssl::SSL_Autoref::kGoal: {
              placing_team = flip_team(update.goal().scoring_team());
              break;
            }
            case ssl::SSL_Autoref::EVENT_NOT_SET: {
              break;
            }
          }
        }

        if (placing_team != TeamNone) {
          board->rcon.sendCommand(
            placing_team == TeamBlue ? SSL_Referee::BALL_PLACEMENT_BLUE : SSL_Referee::BALL_PLACEMENT_YELLOW,
            board->comparer.getDesignatedPoint());
          printf("ball placement: <%.0f,%.0f>\n",
                 board->comparer.getDesignatedPoint().x(),
                 board->comparer.getDesignatedPoint().y());
        }
      }
    }
  });
  call_text = makeText(this, ws(call_str), call_colour);

  sizer->Add(call_text);
  sizer->Add(place_button, 0, wxALIGN_CENTER_VERTICAL);
  sizer->Add(apply_button, 0, wxALIGN_CENTER_VERTICAL);

  if (update.game_timestamp().has_stage_time_left()) {
    stage_time_label = makeText(this, "Stage time left");
    stage_time_text = makeText(this, formatTime(update.game_timestamp().stage_time_left()));
    sizer->Add(stage_time_label);
    sizer->Add(stage_time_text);
  }

  setEmph(emph);

  sizer->Layout();

  animate_timer = new wxTimer(this);
  Bind(wxEVT_TIMER, &wxRefereeHistoryItem::OnTimer, this);
  animate_timer->Start(50);
}

void wxRefereeHistoryItem::setEmph(bool e)
{
  emph = e;

  wxFont label_font(wxFontInfo(20).Family(wxFONTFAMILY_DEFAULT).Italic());
  wxFont text_font(wxFontInfo(25).Family(wxFONTFAMILY_DEFAULT));
  wxFont command_font(wxFontInfo(30).Family(wxFONTFAMILY_DEFAULT));
  wxFont next_command_font(wxFontInfo(40).Family(wxFONTFAMILY_DEFAULT));

  wxFont small_label_font(wxFontInfo(12).Family(wxFONTFAMILY_DEFAULT).Italic());
  wxFont small_text_font(wxFontInfo(20).Family(wxFONTFAMILY_DEFAULT));
  wxFont small_command_font(wxFontInfo(20).Family(wxFONTFAMILY_DEFAULT));
  wxFont small_next_command_font(wxFontInfo(30).Family(wxFONTFAMILY_DEFAULT));

  if (!emph) {
    animate_timer->Stop();
    SetBackgroundColour(wxNullColour);
  }
  else {
    SetBackgroundColour(start_colour);
  }

  if (description_label != nullptr) {
    description_label->SetFont(emph ? label_font : small_label_font);
  }
  if (call_label != nullptr) {
    call_label->SetFont(emph ? label_font : small_label_font);
  }
  if (next_label != nullptr) {
    next_label->SetFont(emph ? label_font : small_label_font);
  }
  if (stage_time_label != nullptr) {
    stage_time_label->SetFont(emph ? label_font : small_label_font);
  }

  if (description_text != nullptr) {
    description_text->SetFont(emph ? text_font : small_text_font);
  }
  if (call_text != nullptr) {
    call_text->SetFont(emph ? command_font : small_command_font);
  }
  if (next_text != nullptr) {
    next_text->SetFont(emph ? next_command_font : small_next_command_font);
  }
  if (stage_time_text != nullptr) {
    stage_time_text->SetFont(emph ? text_font : small_text_font);
  }
}

void printColour(wxColour c)
{
  printf("%d %d %d\n", c.Red(), c.Green(), c.Blue());
}

void wxRefereeHistoryItem::OnTimer(wxTimerEvent &event)
{
  wxColour endColour = wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND);
  uint64_t dt = GetTimeMicros() - create_time;
  double factor = std::min(1., dt / (3 * 1e6));
  SetBackgroundColour(interpolateColour(start_colour, endColour, factor));
}

wxRefereeHistoryItem::~wxRefereeHistoryItem()
{
  delete animate_timer;
}
