#include "StartGame.h"

#include "ClientRequest.h"
#include "Ship.h"
#include "uuid.h"
#include <utility>
#include <vector>

StartGame::StartGame(uuid playerId, std::vector<Ship> ships)
    : ClientRequest(playerId, RequestType::StartGame), m_ships(std::move(ships)) {}

auto StartGame::getShips() const -> std::vector<Ship> {
  return m_ships;
}