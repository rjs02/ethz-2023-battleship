#include "MainGamePanel.h"

#include "../GameController.h"
#include "../uiElements/EmotePanel.h"
#include "Logger.h"


MainGamePanel::MainGamePanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
  // set minimum panel size
  parent->SetMinSize(wxSize(1200, 800));

  buildEmoteList();
}

void MainGamePanel::buildGameState(GameState * gameState, uuid ownId) {
  _gameState = gameState;
  _ownId = ownId;
  _currentPlayer = gameState->getCurrentPlayerId();
  
  wxColor backgroundColor = wxColor(255, 255, 255);
  this->SetBackgroundColour(backgroundColor);
  
  this->DestroyChildren();

  buildEmoteList();

  // this contains the grids and the ships
  wxBoxSizer* gameWindow = new wxBoxSizer(wxVERTICAL);
  // this shows who's turn it is via text
  wxBoxSizer* turnIndicator = new wxBoxSizer(wxVERTICAL);
  // this contains both the grids and the shipPanels
  wxBoxSizer* grids = new wxBoxSizer(wxHORIZONTAL);
  // this contains the own viewGrid and shipPanels
  wxBoxSizer* leftSide = new wxBoxSizer(wxVERTICAL);
  // this contains the opponent's viewGrid and shipPanels
  wxBoxSizer* rightSide = new wxBoxSizer(wxVERTICAL);

  // get player grids
  auto playerGrid = gameState->getPlayerGrid(_ownId);

  // get ships
  std::vector<Ship> playerShips = playerGrid.shipsPlaced;
  // std::vector<Ship> oppShips = oppGrid.m_shipsPlaced;
  bool ownShipSunk[5] = {false, false, false, false, false};
  for (int i=0; i<5; ++i) {
    if (playerShips[i].hasSunken()) {
      ownShipSunk[i] = true;
    }
  }

  for (int i=0; i<5; ++i) {
    if (gameState->getOppShipSunk()[i]) {
      LOG("Opponten ship" + std::to_string(i) + " has sunk");
    } else {
      LOG("Opponten ship" + std::to_string(i) + " has not sunk");
    }
  }

  // get shots
  auto ownShots = playerGrid.shotsFired;
  auto oppShots = playerGrid.shotsReceived;

  this->_ownShipPanel = new ShipPanel(this, wxPoint( 10+110, 460-3+10), ownShipSunk);
  // TODO: find a way to do this without requiring acces to opponent ship vector
  this->_oppShipPanel = new ShipPanel(this, wxPoint(430+110, 460-3+10), gameState->getOppShipSunk());

  this->_ownViewGrid  = new ViewGrid( this, wxPoint( 10+110,  40-3+10), ViewGrid::gridtype::own);
  this->_oppViewGrid  = new ViewGrid( this, wxPoint(430+110,  40-3+10), ViewGrid::gridtype::opp);
  
  this->_ownViewGrid->showShips(playerShips);

  this->_ownViewGrid->showShots(oppShots);
  this->_oppViewGrid->showShots(ownShots);

  this->_oppViewGrid->Bind(wxEVT_LEFT_DOWN, &MainGamePanel::onMouseClick, this);

  wxStaticText* leftTitle = new wxStaticText(this, wxID_ANY, "Your Ships");
  leftSide->Add(leftTitle, 0, wxBOTTOM | wxALIGN_CENTER, 10);

  leftSide->Add(_ownViewGrid, 0, wxDEFAULT, 0);
  leftSide->Add(_ownShipPanel, 0, wxDEFAULT, 10);

  wxStaticText* rightTitle = new wxStaticText(this, wxID_ANY, "Opponent's Ships");
  rightSide->Add(rightTitle, 0, wxALIGN_CENTER | wxBOTTOM, 10);

  rightSide->Add(_oppViewGrid, 0, wxDEFAULT, 0);
  rightSide->Add(_oppShipPanel, 0, wxDEFAULT, 0);

  // place grids next to eachother
  grids->Add(leftSide, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
  grids->Add(rightSide, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);

  buildTurnIndicator(_gameState->getPlayer(_currentPlayer).getName(), turnIndicator);
  gameWindow->Add(turnIndicator, 0, wxALIGN_CENTER, 0);
  gameWindow->Add(grids, 0, wxALIGN_CENTER, 10);

  _mainWindow->Add(gameWindow, 0, wxALL, 10);

  this->SetSizerAndFit(_mainWindow);
}

void MainGamePanel::buildEmoteList() {
  // this box contains everything
  _mainWindow = new wxBoxSizer(wxHORIZONTAL);
  // box for the emote panel
  _emoteWindow = new wxBoxSizer(wxVERTICAL);

  this->_emotePanel = new EmotePanel(this, wxPoint(10, 10));

  _emoteWindow->Add(_emotePanel, 0, wxALIGN_CENTER, 0);

  _mainWindow->Add(_emoteWindow, 0, wxALIGN_CENTER, 0);

  this->_emotePanel->Bind(wxEVT_LEFT_DOWN, &MainGamePanel::onEmoteClick, this);
}

void MainGamePanel::displayEmote(EmoteType emote) {
  int emoteId = static_cast<int>(emote);
  LOG("emote id: " + std::to_string(emoteId));
  if (emoteId < 0 || emoteId > 2) {
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
  }
  _currentEmote = new wxStaticBitmap(this, wxID_ANY, wxBitmap(image), wxPoint(120, 47), wxSize(400, 400), 0);
  // for some reason the image is shown and removed at the next panel update
  _currentEmote = nullptr;
}

void MainGamePanel::buildTurnIndicator(std::string playerName, wxBoxSizer *box) {
  auto text = "It's " + playerName + "'s turn";
  wxStaticText* turnText = new wxStaticText(this, wxID_ANY, text);
  box->Add(turnText, 0, wxALIGN_TOP, 0);
}

void MainGamePanel::onMouseClick(wxMouseEvent &event) {
  // only allow clicks if it is the player's turn
  if (_currentPlayer != _ownId) {
    LOG("not your turn");
    return;
  }
  // temp test
  //AudioPlayer::play(AudioPlayer::BestPirate);
  // end test

  wxPoint mousePosition = event.GetPosition();
  // mousePosition -= _oppViewGrid->GetPosition();
  
  Coordinate shot = Coordinate{mousePosition.x / 40, mousePosition.y / 40};

  LOG("clicked on cell " + std::to_string(shot.x) + ", " + std::to_string(shot.y));

  // limit shot frequency to one per second to give server time to respond
  auto now = std::chrono::system_clock::now();
  if (now - _lastClick < std::chrono::milliseconds(57)) {
    LOG("too fast");
    return;
  }
  _lastClick = now;

  if (!_gameState->shotIsLegal(_ownId, shot)) {
    LOG("illegal shot");
    return;
  }

  GameController::callShot(shot); // builds callShot request and sends to server
}


void MainGamePanel::onEmoteClick(wxMouseEvent &event) {
  auto now = std::chrono::system_clock::now();
  if (now - _lastClick < std::chrono::milliseconds(57)) {
    LOG("too fast");
    return;
  }
  _lastClick = now;
  wxPoint mousePosition = event.GetPosition();
  mousePosition -= wxPoint(20, 90);
  const int emote = mousePosition.y / 90;
  if(emote < 0 || emote > 2) return;
  LOG("clicked on emote nr " + std::to_string(emote));
  GameController::sendEmote(EmoteType(emote));
}
