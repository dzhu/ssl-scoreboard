#include "gui_utils.h"
#include "shared/util.h"
#include "style.h"

#include "history_item.h"

using gbp = wxGBPosition;

std::string infringementDescription(ssl::SSL_Autoref::RuleInfringement::FoulType f)
{
  switch (f) {
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

wxRefereeHistoryItem::wxRefereeHistoryItem(wxWindow *parent, wxWindowID id, ssl::SSL_Autoref update)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED),
      emph(true),
      call_label(nullptr),
      call_text(nullptr),
      next_label(nullptr),
      next_text(nullptr),
      description_label(nullptr),
      description_text(nullptr),
      stage_time_label(nullptr),
      stage_time_text(nullptr)
{
  create_time = GetTimeMicros();

  auto sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(sizer);

  sizer->Add(100, 0);

  call_label = makeText(this, "Call");
  sizer->Add(call_label);

  std::string call_str, robots_str;
  wxColour call_colour;

  switch (update.event_case()) {
    case ssl::SSL_Autoref::kBallOutOfField: {
      bool isBlue = update.ball_out_of_field().last_touch() == ssl::SSL_Autoref_Team_BLUE;
      call_str = "Ball out";
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;

      start_colour.Set(255, 128, 128);
    } break;
    case ssl::SSL_Autoref::kFoul: {
      bool isBlue = update.foul().offending_team() == ssl::SSL_Autoref_Team_BLUE;
      call_str = "Foul: " + infringementDescription(update.foul().foul_type());
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;
      start_colour.Set(255, 128, 128);
    } break;
    case ssl::SSL_Autoref::kGoal: {
      bool isBlue = update.goal().scoring_team() == ssl::SSL_Autoref_Team_BLUE;
      call_str = "Goal!";
      call_colour = isBlue ? blue_team_colour : yellow_team_colour;

      if (update.goal().has_scoring_robot()) {
      }

      start_colour.Set(32, 255, 32);
    } break;
    case ssl::SSL_Autoref::kLackOfProgress:
      // TODO
      break;
    case ssl::SSL_Autoref::EVENT_NOT_SET:
      break;
  }

  call_text = makeText(this, ws(call_str), call_colour);
  sizer->Add(call_text);

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
  wxFont text_font(wxFontInfo(40).Family(wxFONTFAMILY_DEFAULT));
  wxFont command_font(wxFontInfo(55).Family(wxFONTFAMILY_DEFAULT));
  wxFont next_command_font(wxFontInfo(55).Family(wxFONTFAMILY_DEFAULT));

  wxFont small_label_font(wxFontInfo(12).Family(wxFONTFAMILY_DEFAULT).Italic());
  wxFont small_text_font(wxFontInfo(20).Family(wxFONTFAMILY_DEFAULT));
  wxFont small_command_font(wxFontInfo(40).Family(wxFONTFAMILY_DEFAULT));
  wxFont small_next_command_font(wxFontInfo(40).Family(wxFONTFAMILY_DEFAULT));

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
