#include "ResponseListenerThread.h"

#include "ClientNetworkManager.h"
#include "GameController.h"
#include "Logger.h"
#include "network/responses/ErrorResponse.h"
#include "network/responses/GameOverEvent.h"
#include <sstream>

ResponseListenerThread::ResponseListenerThread(sockpp::tcp_connector *connection) {
  this->_connection = connection;
}

ResponseListenerThread::~ResponseListenerThread() {
  this->_connection->shutdown();
}

wxThread::ExitCode ResponseListenerThread::Entry() {
  try {
    char    buffer[512]; // 512 bytes
    ssize_t count = 0;

    while ((count = this->_connection->read(buffer, sizeof(buffer))) > 0) {
      LOG("Client listening...");
      try {
        int pos = 0;

        // extract length of message in bytes (which is sent at the start of the message, and is separated by a ":")
        std::stringstream messageLengthStream;
        while (buffer[pos] != ':' && pos < count) {
          messageLengthStream << buffer[pos];
          pos++;
        }
        ssize_t messageLength = std::stoi(messageLengthStream.str());

        // initialize a stream for the message
        std::stringstream messageStream;

        // copy everything following the message length declaration into a stringstream
        messageStream.write(&buffer[pos + 1], count - (pos + 1));
        ssize_t bytesReadSoFar = count - (pos + 1);

        // read remaining packages until full message length is reached
        while (bytesReadSoFar < messageLength && count != 0) {
          count = this->_connection->read(buffer, sizeof(buffer));
          messageStream.write(buffer, count);
          bytesReadSoFar += count;
        }

        // process message (if we've received entire message)
        if (bytesReadSoFar == messageLength) {
          LOG("Received entire message...");
          std::string                     message  = messageStream.str();
          std::unique_ptr<ServerResponse> response = ClientNetworkManager::parseResponse(message);

          LOG("Response type: " + std::to_string(static_cast<int>(response->responseType)));

          switch (response->responseType) {

          case ResponseType::GameEvent: {
            LOG("received a GameEvent");
            const GameEvent &gameEvent = static_cast<const GameEvent &>(*response);
            GameController::getMainThreadEventHandler()->CallAfter([gameEvent] {
              GameController::handleGameEvent(gameEvent);
            });
            break;
          }
          case ResponseType::EmoteEvent: {
            LOG("received an EmoteEvent");
            const EmoteEvent &emoteEvent = static_cast<const EmoteEvent &>(*response);
            GameController::getMainThreadEventHandler()->CallAfter([emoteEvent] {
              GameController::showEmote(emoteEvent);
            });
            break;
          }
          case ResponseType::JoinGameSuccess: {
            LOG("received a JoinGameSuccess");
            GameController::getMainThreadEventHandler()->CallAfter([] {
              GameController::enterSetupPhase();
            });
          } break;
          case ResponseType::ErrorResponse: {
            const ErrorResponse &errorResponse = static_cast<const ErrorResponse &>(*response);
            GameController::getMainThreadEventHandler()->CallAfter([errorResponse] {
              GameController::showError("Server Error", errorResponse.exception.what(), false);
            });
          } break;
          case ResponseType::StartGameSuccess: {
            LOG("received a StartGameSuccess");
            const StartGameSuccess &startGameSuccess = static_cast<const StartGameSuccess &>(*response);
            GameController::getMainThreadEventHandler()->CallAfter([startGameSuccess] {
              GameController::startGame(startGameSuccess);
            });
          } break;
          case ResponseType::QuitGameEvent: {
            LOG("received a QuitGameEvent");
            const QuitGameEvent &quitGameEvent = static_cast<const QuitGameEvent &>(*response);
            LOG("player " + quitGameEvent.quitPlayerId.ToString() + " quit the game");
            GameController::getMainThreadEventHandler()->CallAfter([quitGameEvent] {
              GameController::handleQuitGameEvent(quitGameEvent.quitPlayerId);
            });
          } break;
          case ResponseType::GameOverEvent: {
            LOG("received a GameOverEvent");
            const GameOverEvent &gameOverEvent = static_cast<const GameOverEvent &>(*response);
            uuid                 winnerId      = gameOverEvent.winnerPlayerId;
            LOG("Player " + winnerId.ToString() + " won the game");
            GameController::getMainThreadEventHandler()->CallAfter([winnerId] {
              GameController::gameOver(winnerId);
            });
          }
          }
        } else {
          this->outputError("Network error", "Could not read entire message. TCP stream ended early. Difference is " +
                                                 std::to_string(messageLength - bytesReadSoFar) + " bytes");
        }

      } catch (std::exception &e) {
        // Make sure the connection isn't terminated only because of a read error
        this->outputError("Network error", "Error while reading message: " + std::string(e.what()));
      }
    }

    if (count <= 0) {
      this->outputError("Network error", "Read error [" + std::to_string(this->_connection->last_error()) +
                                             "]: " + this->_connection->last_error_str());
    }

  } catch (const std::exception &e) {
    this->outputError("Network error", "Error in listener thread: " + std::string(e.what()));
  }

  this->_connection->shutdown();

  return static_cast<wxThread::ExitCode>(0); // everything okay
}

void ResponseListenerThread::outputError(std::string title, std::string message) {
  // TODO GameController getMainThreadEventHandler
  //   GameController::getMainThreadEventHandler()->CallAfter([title, message] {
  //     GameController::showError(title, message);
  //   });
}
