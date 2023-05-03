//
// Created by Tejas Gupta on 4/19/23.
//

#ifndef BATTLESHIP_GAME_INSTANCE_H
#define BATTLESHIP_GAME_INSTANCE_H

#include <mutex>
#include <string>
#include <vector>

#include "game_state/Player.h"
#include "game_state/game_state.h"
#include "network/requests/CallShot.h"

class game_instance {

private:
  game_state              *_game_state;
  bool                     is_player_allowed_to_play(Player *player);
  inline static std::mutex modification_lock;

public:
  game_instance();
  ~game_instance() {
    if (_game_state != nullptr) {
      delete _game_state;
    }
    _game_state = nullptr;
  }
  std::string get_id();

  game_state *get_game_state();

  bool is_full();
  bool is_started();
  bool is_finished();

  // game update functions
  bool start_game(Player *player, std::string &err);
  bool try_add_player(Player *new_player, std::string &err);
  bool try_remove_player(Player *player, std::string &err);
  // TODO: add call_shot, play_again,
  bool execute_shot(Player *player, CallShot);  // registers a shot and checks if it's a hit or miss and if it sunk a ship. notifies clients accordingly by emitting game events.
  //  bool play_card(Player* player, const std::string& card_id, std::string& err);
  //  bool draw_card(Player* player, card*& drawn_card, std::string& err);
  //  bool fold(player* Player, std::string& err);
};

#endif // BATTLESHIP_GAME_INSTANCE_H
