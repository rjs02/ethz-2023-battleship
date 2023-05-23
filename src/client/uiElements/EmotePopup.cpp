#include "EmotePopup.h"

EmotePopup::EmotePopup(wxWindow *parent, wxPoint pos, EmoteType emote) : wxPopupWindow(parent) {
    this->SetSize(400, 400);
    wxColor backgroundColor = wxColor(255, 0, 255);
    this->SetBackgroundColour(backgroundColor);
    int emoteId = static_cast<int>(emote);
    if (emoteId < 0 || emoteId > 3) {
        LOG("invalid emote id");
        return;
    }
    LOG("I should display emote " + std::to_string(emoteId));
    wxImage image;
    switch (emoteId) {
    case 0:
        image = wxImage("../assets/emotes/large_middlefinger.png");
        break;
    case 1:
        image = wxImage("../assets/emotes/large_gofuckyourself.png");
        break;
    case 2:
        image = wxImage("../assets/emotes/large_mocking.png");
        break;
    case 3:
        image = wxImage("../assets/emotes/large_bestpirate.png");
        break;
    }
    _currentEmote = new wxStaticBitmap(this, wxID_ANY, wxBitmap(image), wxPoint(0, 0), wxSize(400, 400), 0);
    _currentEmote->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent &event) {
        LOG("Hide current emote");
        // one of these ought to do the trick
        this->_currentEmote->Hide();
        this->Hide();
        this->Destroy();
    });
}