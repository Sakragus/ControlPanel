#pragma once

#include <Arduino.h>
#include <Game.h>
#include <Obstacle.h>
#include <TM1637Plus.h>
#include <TM1638Plus.h>

class Game {
  private:
    byte difficulty;
    byte prevPos;
    int score;
    int displayScore;
    int updateTimeMs;
    int collisionCounter;
    int maxCollisionCounter;
    const int OBSTACLES_SIZE;
    const int MAX_DISPLAY_SCORE = 9999 * 2;
    unsigned long lastGameUpdateTime;
    Lane playerLane;
    GameState gameState;
    char scoreBuffer[5] = {};
    Obstacle obstacles[9];
    TM1638plus gameDisp;
    TM1637plus_modelX scoreDisp;

  public:
    Game(int score, byte difficulty, Lane playerLane, GameState gameState, TM1638plus gameDisp,
         TM1637plus_modelX scoreDisp);

    unsigned long getLastGameUpdatTime() const;

    void setLastGameUpdateTime(unsigned long lastGameUpdateTime);

    int getMaxDisplayScore() const;

    int getScore() const;

    void setScore(int score);

    int getDisplayScore() const;

    void setDisplayScore(int score);

    int getCollisionCounter() const;

    void setCollisionCounter(int collisionCounter);

    int getMaxCollisionCounter() const;

    void setMaxCollisionCounter(int maxCollisionCounter);

    unsigned long getGameUpdateTimeMs();

    byte getDifficulty() const;

    void setDifficulty(byte difficulty);

    Lane getPlayerLane() const;

    void setPlayerLane(Lane lane);

    GameState getGameState() const;

    void setGameState(GameState gameState);

    int getObstaclesSize() const;

    Lane getRandLane();

    bool checkCollision();

    Obstacle getObstacle(int index);

    void setObstacle(int index, Obstacle obstacle);

    void clearObstacles();

    void shiftObstaclesLeft();

    void moveUp();

    void moveDown();

    void start();

    void reset();

    void update(unsigned long currentTime);
};