#include "GameController.h"
#include "AudioPlayer.h"
#include "ClientNetworkManager.h"
#include "Logger.h"
#include "network/requests/CallShot.h"
#include "network/requests/JoinGame.h"
#include "network/requests/QuitGame.h"
#include "network/requests/SendEmote.h"
#include "network/requests/StartGame.h"

// initialize static variables
GameWindow                                        *GameController::_gameWindow      = nullptr;
ConnectionPanel                                   *GameController::_connectionPanel = nullptr;
SetupPanel                                        *GameController::_setupPanel      = nullptr;
MainGamePanel                                     *GameController::_mainGamePanel   = nullptr;
SetupManager                                      *GameController::_setupManager    = nullptr;
Player                                            *GameController::_me              = nullptr;
GameState                                         *GameController::_gameState       = nullptr;
std::chrono::time_point<std::chrono::system_clock> GameController::_lastClick;

void GameController::init(GameWindow *gameWindow) {
  LOG("initializing controller");
  GameController::_gameWindow = gameWindow;

  // setup panels
  GameController::_connectionPanel = new ConnectionPanel(gameWindow);
  GameController::_setupPanel      = new SetupPanel(gameWindow);
  GameController::_mainGamePanel   = new MainGamePanel(gameWindow);

  // hide panels
  GameController::_connectionPanel->Show(false);
  GameController::_setupPanel->Show(false);
  GameController::_mainGamePanel->Show(false);

  // start of game: show connection panel
  GameController::_gameWindow->showPanel(GameController::_connectionPanel);

  _lastClick = std::chrono::system_clock::now();
}

void GameController::connectToServer() {
  // get values
  wxString inputServerAddress = GameController::_connectionPanel->getServerAddress().Trim();
  wxString inputServerPort    = GameController::_connectionPanel->getServerPort().Trim();
  wxString inputPlayerName    = GameController::_connectionPanel->getUserName().Trim();

  // check that all values were provided
  if (inputServerAddress.IsEmpty()) {
    GameController::showError("Input error", "Please provide the server's address", true);
    return;
  }
  if (inputServerPort.IsEmpty()) {
    GameController::showError("Input error", "Please provide the server's port number", true);
    return;
  }
  if (inputPlayerName.IsEmpty()) {
    GameController::showError("Input error", "Please enter your desired player name", true);
    return;
  }

  // convert host from wxString to std::string
  std::string host = inputServerAddress.ToStdString();

  // convert port from wxString to uint16_t
  unsigned long portAsLong;
  if (!inputServerPort.ToULong(&portAsLong) || portAsLong > 65535) {
    GameController::showError("Connection error", "Invalid port", true);
    return;
  }
  uint16_t port = static_cast<uint16_t>(portAsLong);

  // convert player name from wxString to std::string
  std::string playerName = inputPlayerName.ToStdString();

  // connect to network
  ClientNetworkManager::init(host, port);

  // send request to join game
  LOG("sending join request");
  GameController::_me = new Player(uuid::generateRandomUuid(), playerName);
  JoinGame request    = JoinGame(GameController::_me->getId(), GameController::_me->getName());

  ClientNetworkManager::sendRequest(request);
}

void GameController::enterSetupPhase() {
  // show setup panel
  GameController::_setupManager = new SetupManager();
  GameController::_gameWindow->showPanel(GameController::_setupPanel);
}

void GameController::startGame(const StartGameSuccess &response) { // called by ResponseListenerThread
  LOG("Game is starting");
  // adding opponent to game state
  assert(response.players.size() == 2);
  Player opponent = response.players.at(0);
  if (opponent.getId() == _me->getId()) {
    opponent = response.players.at(1);
  }
  _gameState->addPlayer(opponent);
  // set starting player
  LOG("Starting game state");
  assert(_gameState != nullptr);
  _gameState->start(response.startingPlayerId);
  // show GUI
  GameController::_gameWindow->showPanel(GameController::_mainGamePanel);
  GameController::_mainGamePanel->buildGameState(GameController::_gameState, GameController::_me->getId());
  GameController::_gameWindow->Layout();
}

void GameController::handleGameEvent(const GameEvent &event) {
  _gameState->updateBoards(event);
  _mainGamePanel->buildGameState(_gameState, _me->getId());
  // remove this condition if playing on 2 devices. This for now avoids playing every sound double resulting in
  // terrible quality. Sound is only played on the shooters end.
  if (event.playerId == _me->getId()) {
    if (event.hit) {
      AudioPlayer::play(AudioPlayer::Cannon);
    } else {
      AudioPlayer::play(AudioPlayer::Miss);
    }
  }
}

void GameController::callShot(Coordinate position) {
  LOG("Calling shot on position " + std::to_string(position.x) + "," + std::to_string(position.y));
  // only current player can shoot
  if (_gameState->getCurrentPlayerId() != _me->getId()) {
    LOG("not your turn");
    return;
  }
  // limit shot frequency to avoid errors due to network delay
  auto now = std::chrono::system_clock::now();
  if (now - _lastClick < std::chrono::milliseconds(100)) { // 57
    LOG("too fast");
    return;
  }
  _lastClick = now;
  // make sure shot is legal
  if (!_gameState->shotIsLegal(_me->getId(), position)) {
    LOG("illegal shot");
    return;
  }
  // shot is ok, send to server
  const CallShot request = CallShot(_me->getId(), position);
  ClientNetworkManager::sendRequest(request);
  // AudioPlayer::play(AudioPlayer::Cannon);
}

void GameController::sendEmote(EmoteType emote) {
  const SendEmote request = SendEmote(_me->getId(), emote);
  ClientNetworkManager::sendRequest(request);
}

void GameController::showEmote(const EmoteEvent &emoteEvent) {
  const EmoteType   emote = emoteEvent.emote;
  const std::string file  = EmoteHandler::getImage(emote);
  _mainGamePanel->displayEmote(emote);
  AudioPlayer::play(EmoteHandler::getSound(emote));
}

void GameController::showError(const std::string &title, const std::string &message, bool popup) {
  std::cout << "ERROR [" << title << "] " << message << std::endl;
  if (popup) {
    wxMessageBox(message, title, wxOK | wxICON_ERROR);
  }
}

void GameController::gameOver(uuid winnerId) {
  _gameState->finish();
  std::string message = "\n";
  if (winnerId == _me->getId()) {
    message += "You won!";
    // AudioPlayer::play(AudioPlayer::Victory);
  } else {
    message += "You lost!";
    // AudioPlayer::play(AudioPlayer::Defeat);
  }
  wxMessageBox(message, "Game Over", wxOK | wxICON_INFORMATION);
  // Send player back to connection panel
  GameController::init(_gameWindow);
}

void GameController::playerReady() {
  if (!_setupManager->placedAllShips()) {
    GameController::showError("Setup error", "Please place all ships before clicking ready", true);
    return;
  }

  // generate GameState
  LOG("generating GameState");
  _gameState = new GameState(GameState::Type::ClientState);
  _gameState->addPlayer(*_me);
  _gameState->addShips(_me->getId(), _setupManager->_ships_placed);
  LOG("printing ship ids...");
  for (auto ship : _gameState->getPlayerGrid(_me->getId()).shipsPlaced) {
    LOG(ship.getId().ToString());
  }

  LOG("sending request to server. You might need to wait for your opponent to be ready.");

  // send request to start game
  const StartGame request = StartGame(_me->getId(), _setupManager->_ships_placed);
  ClientNetworkManager::sendRequest(request);

  // disable button such that player cannot click it again
  GameController::_setupPanel->getReadyButton()->Disable();

  // display text that ship placement was submitted and that player is waiting for opponent
  GameController::_setupPanel->getReadyText()->SetLabel("Ship placement submitted.\nWaiting for opponent...");
  GameController::_setupPanel->Layout();
}

wxEvtHandler *GameController::getMainThreadEventHandler() {
  return GameController::_gameWindow->GetEventHandler();
}

void GameController::quitGame() {
  // send quit game request
  LOG("Sending quit game request");
  const QuitGame request = QuitGame(_me->getId());
  ClientNetworkManager::sendRequest(request);
}

void GameController::handleQuitGameEvent(uuid quitterId) {
  if (quitterId != _me->getId()) {
    const std::string message = "Your opponent left the game\n";
    wxMessageBox(message, "Opponent left", wxOK | wxICON_INFORMATION);
    LOG("resetting client...");
    GameController::init(_gameWindow);
  }
}
