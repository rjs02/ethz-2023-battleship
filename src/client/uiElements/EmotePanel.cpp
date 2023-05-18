#include "EmotePanel.h"
#include "Logger.h"

#include <iostream>

EmotePanel::EmotePanel(wxWindow *parent, wxPoint pos) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(100, 600)) {
    _parent = parent;
    // wxColor backgroundColor = wxColor(255, 0, 255);
    // this->SetBackgroundColour(backgroundColor);
    wxBitmap emoteBitmaps[3] = {
        wxBitmap(wxImage(EmoteHandler::getImage(EmoteType::MiddleFinger))),
        wxBitmap(wxImage(EmoteHandler::getImage(EmoteType::RussianWarshipGoFuckYourself))),
        wxBitmap(wxImage(EmoteHandler::getImage(EmoteType::Mocking)))
    };
    for (int i=0; i<3; ++i) {
        _emoteBitmaps[i] = new wxStaticBitmap(_parent, wxID_ANY, emoteBitmaps[i], wxPoint(20, 90 + 90*i), wxSize(80, 80), 0);
    }
    // displayEmote(EmoteType::MiddleFinger);
}
