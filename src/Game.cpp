#include <Game.h>
#include <TM1637Plus.h>
#include <TM1638Plus.h>

Game::Game(int score, byte difficulty, Lane playerLane, GameState gameState, TM1638plus gameDisp,
           TM1637plus_modelX scoreDisp)
    : score(score), difficulty(difficulty), playerLane(playerLane), gameState(gameState),
      prevPos(0), OBSTACLES_SIZE(9), obstacles{Obstacle()}, gameDisp(gameDisp),
      scoreDisp(scoreDisp) {}

unsigned long Game::getLastGameUpdatTime() const { return lastGameUpdateTime; }

void Game::setLastGameUpdateTime(unsigned long lastGameUpdateTime) {
    this->lastGameUpdateTime = lastGameUpdateTime;
}

int Game::getScore() const { return score; }

void Game::setScore(int score) {
    if (score < 0) {
        score = 0;
    }

    this->score = score;
    setDisplayScore(score);
}

int Game::getDisplayScore() const { return displayScore; }

void Game::setDisplayScore(int displayScore) {
    if (score > 9999) {
        displayScore = score - 9999;
    }

    this->displayScore = displayScore;
}

int Game::getCollisionCounter() const { return collisionCounter; }

void Game::setCollisionCounter(int collisionCounter) {
    collisionCounter = collisionCounter < 1 ? 0 : collisionCounter;
    this->collisionCounter = collisionCounter;
}

unsigned long Game::getGameUpdateTimeMs() { return 1000 - (90 * getDifficulty()); }

int Game::getMaxCollisionCounter() const { return maxCollisionCounter; }

void Game::setMaxCollisionCounter(int maxCollisionCounter) {
    this->maxCollisionCounter = maxCollisionCounter;
}

byte Game::getDifficulty() const { return difficulty; }

void Game::setDifficulty(byte difficulty) {
    if (difficulty < 0 || difficulty == 255) {
        difficulty = 0;
    } else if (difficulty > 8) {
        difficulty = 8;
    }

    this->difficulty = difficulty;
}

Lane Game::getPlayerLane() const { return playerLane; }

void Game::setPlayerLane(Lane lane) { playerLane = lane; }

GameState Game::getGameState() const { return gameState; }

void Game::setGameState(GameState gameState) { this->gameState = gameState; }

void Game::moveUp() {
    switch (playerLane) {
    case Lane::BOTTOM:
        playerLane = Lane::MIDDLE;
        break;

    case Lane::MIDDLE:
        playerLane = Lane::TOP;
        break;

    default:
        break;
    }
}

void Game::moveDown() {
    switch (playerLane) {
    case Lane::MIDDLE:
        playerLane = Lane::BOTTOM;
        break;

    case Lane::TOP:
        playerLane = Lane::MIDDLE;
        break;

    default:
        break;
    }
}

int Game::getObstaclesSize() const { return OBSTACLES_SIZE; }

Obstacle Game::getObstacle(int index) { return obstacles[index]; }

void Game::setObstacle(int index, const Obstacle obstacle) {
    if (index < getObstaclesSize()) {
        obstacles[index] = obstacle;
    }
}

Lane Game::getRandLane() {
    Lane lane;
    switch (rand() % 4) {
    case 0:
        lane = Lane::BOTTOM;
        break;
    case 1:
        lane = Lane::MIDDLE;
        break;
    case 2:
        lane = Lane::TOP;
        break;
    case 3:
        lane = Lane::NONE;
        break;

    default:
        break;
    }

    return lane;
}

void Game::shiftObstaclesLeft() {
    for (int i = 0; i < getObstaclesSize() - 1; i++) {
        obstacles[i] = obstacles[i + 1];
    }

    obstacles[getObstaclesSize() - 1] = Obstacle(getRandLane());
}

void Game::clearObstacles() {
    for (int i = 0; i < getObstaclesSize() - 1; i++) {
        obstacles[i] = Obstacle();
    }
}

bool Game::checkCollision() {
    Lane gap = getObstacle(0).getGap();
    Serial.print("Gap: ");
    Serial.print(static_cast<int>(gap));
    Serial.print(" Player: ");
    Serial.print(static_cast<int>(getPlayerLane()));

    if (gap != getPlayerLane() && gap != Lane::NONE) {
        Serial.println(" COLLISION");
    } else
        Serial.println();

    return gap != getPlayerLane() && gap != Lane::NONE;
}

int Game::getMaxDisplayScore() const { return MAX_DISPLAY_SCORE; }

void Game::update(unsigned long currentTime) {

    if (getGameState() == GameState::PLAYING &&
        currentTime - lastGameUpdateTime > getGameUpdateTimeMs()) {

        lastGameUpdateTime = currentTime;

        for (int i = 1; i < getObstaclesSize(); i++) {
            gameDisp.display7Seg(i - 1, getObstacle(i).getBinaryValue());
        }

        shiftObstaclesLeft();

        if (getObstacle(0).getGap() != Lane::NONE && getGameState() == GameState::PLAYING) {
            setScore(getScore() + 1);
        }

        if (getScore() >= MAX_DISPLAY_SCORE) {
            setGameState(GameState::GAME_WON);
            Serial.print("Game Over. You won. Final Score: ");
            Serial.println(getScore());
        }

        sprintf(scoreBuffer, "%04d", getDisplayScore());
        scoreDisp.DisplayString(scoreBuffer, getScore() >= MAX_DISPLAY_SCORE ? 0x40 : 0, 4, 0);
    }

    if (getGameState() == GameState::PLAYING) {
        switch (getPlayerLane()) {
        case Lane::BOTTOM:
            gameDisp.display7Seg(0,
                                 0b00001000 | Obstacle(getObstacle(0).getGap()).getBinaryValue());
            break;
        case Lane::MIDDLE:
            gameDisp.display7Seg(0,
                                 0b01000000 | Obstacle(getObstacle(0).getGap()).getBinaryValue());
            break;
        case Lane::TOP:
            gameDisp.display7Seg(0,
                                 0b00000001 | Obstacle(getObstacle(0).getGap()).getBinaryValue());
            break;
        default:
            break;
        }

        if (checkCollision()) {

            setCollisionCounter(getCollisionCounter() + 1);

            if (getCollisionCounter() > getMaxCollisionCounter()) {
                setGameState(GameState::GAME_OVER);

                Serial.println("GAME OVER!");

                for (int i = 0; i < getObstaclesSize(); i++) {
                    gameDisp.display7Seg(i, 0b01000000);
                }
            }
        } else {
            setCollisionCounter(getCollisionCounter() - 1);
        }
    }
}

void Game::reset() {
    setGameState(GameState::READY);
    clearObstacles();
    setScore(0);
    setCollisionCounter(0);
}

void Game::start() {
    scoreDisp.displayBegin();
    scoreDisp.setBrightness(7, true);
    setGameState(GameState::PLAYING);
    setMaxCollisionCounter(map(getDifficulty(), 0, 8, 180, 80));
}