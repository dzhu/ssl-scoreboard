#include "history_panel.h"
#include "history_item.h"

void HistoryPanel::update(const ssl::SSL_Autoref &a)
{
  auto sizer = GetSizer();

  // set current first item to small
  if (sizer->GetItemCount() > 0) {
    wxSizerItem *first_item = sizer->GetItem(static_cast<size_t>(0));
    wxRefereeHistoryItem *win = (wxRefereeHistoryItem *)(first_item->GetWindow());
    win->setEmph(false);
  }

  // create and insert new item
  wxRefereeHistoryItem *item = new wxRefereeHistoryItem(this, wxID_ANY, a);
  //sizer->PrependSpacer(10);
  sizer->Insert(0, item, 0, wxEXPAND);

  // remove oldest item, if too many
  if (sizer->GetItemCount() > max_items) {
    sizer->Remove(max_items);
  }

  sizer->Layout();
}
