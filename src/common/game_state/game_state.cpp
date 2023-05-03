#include "game_state.h"
#include "Player.h"
#include <utility>

auto game_state::addPlayer(Player player) -> bool {
  // TODO check if player is alrady added
  if (m_players.size() >= 2) {
    return false;
  }
  m_players.push_back(std::move(player));
  return true;
}

auto game_state::addShips(uuid playerId, std::vector<Ship> shipPlacement) -> bool {
  // TODO check for valid playerid
  // TODO check if not already placed
  m_playerGrid.emplace_back(playerId, std::move(shipPlacement));
  return true;
}