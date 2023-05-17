//
// Created by Tejas Gupta on 4/19/23.
//

#include "game_instance.h"

#include "network/responses/ErrorResponse.h"
#include "network/responses/GameEvent.h"
#include "network/responses/JoinGameSuccess.h"
#include "network/responses/ServerResponse.h"
#include "server_network_manager.h"

bool game_instance::joinGame(const JoinGame &joinGameRequest) {
  std::cout << "about to add player to the game state\n";
  std::lock_guard<std::mutex> lockGuard(modification_lock);

  uuid        playerId   = joinGameRequest.getPlayerId();
  std::string playerName = joinGameRequest.getPlayerName();
  Player      player     = Player(playerId, playerName);

  //  // Prints out the names of the current players in the game
  //  std::cout << "current players in the game:\n";
  //  auto players = _game_state.get_players();
  //  for (const auto& player : players) {
  //    std::cout << "Player name: " << player.getName() << std::endl;
  //  }

  return _game_state.addPlayer(player); // addPlayer checks if the player was already added or game is full
}

// is called upon receiving a startGameRequest
// first boolean returns whether start_game was successful
// second boolean returns whether both players are ready to start the game
bool game_instance::start_game(const Player *player, std::string &err) {

  std::lock_guard<std::mutex> lockGuard(modification_lock);
  std::cout << "start_game called" << std::endl;
  const std::vector<Player> currentPlayers = _game_state.get_players();

  // set this player to ready. not a problem if done more than once
  isReady[player->getId()] = true;
  std::cout << player->getId().ToString() << " is ready" << std::endl;

  // check if we have 2 players
  if (currentPlayers.size() != 2) {
    std::cout << "Not 2 players yet. Cannot start game" << std::endl;
    return false;
  }

  // check if other player is ready
  const Player &otherPlayer = _game_state.getOtherPlayer(player->getId());
  if (!isReady[otherPlayer.getId()]) {
    std::cout << "Other player " + otherPlayer.getId().ToString() + " not ready yet. Cannot start game" << std::endl;
    return false;
  }

  // if reached here everything is fine and we can start
  std::cout << "got here " << std::endl;
  return _game_state.start(player->getId()); // second player to press ready is first player to play for the moment
}

bool game_instance::executeShot(CallShot shotRequest) {

  // variables to be determined by game_state
  bool  hit;          // indicates if the shot was a hit
  Ship *hitShip;      // the ship that was hit (if there was a hit)
  bool  sunk;         // indicates if a ship was sunk
  uuid  nextPlayerId; // player who goes next

  // register shot and get results
  bool success = _game_state.registerShot(shotRequest.getPlayerId(), shotRequest.getPosition(), &hit, &hitShip, &sunk, &nextPlayerId);

  // build game event
  GameEvent *shotCalled = new GameEvent(shotRequest.getPlayerId(), shotRequest.getPosition(), hit, sunk, *hitShip, nextPlayerId);
  ServerResponse *msg_string = shotCalled;
  // broadcast to clients
  server_network_manager::broadcast_message(*msg_string, _game_state.get_players());

  if (_game_state.gameOver()) {
    uuid winnerId = _game_state.getWinner();
    // TODO send GameOver response to clients
  }
  return success;
}

game_state &game_instance::getGameState() {
  return _game_state;
}

// TODO remove because never used. functionality is implemented in game_instance::joinGame
/*bool game_instance::try_add_player(Player *new_player, std::string &err) {
  modification_lock.lock();
  if (_game_state.addPlayer(*new_player)) {
    // respond with JoinGameSuccess response
    JoinGameSuccess *msg_string = new JoinGameSuccess();
    server_network_manager::broadcast_message(*msg_string, _game_state.get_players(), new_player);
    modification_lock.unlock();
    return true;
  }
  modification_lock.unlock();
  return false;
}*/
