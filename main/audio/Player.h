/*
 * Player.h
 *
 *  Created on: 27 de jul de 2018
 *      Author: makara
 */

#ifndef MAIN_AUDIO_PLAYER_H_
#define MAIN_AUDIO_PLAYER_H_

class Player {
public:
  static Player* get();
  void setup_triangle_sine_waves(int bits);
  void play();
private:
  int test_bits=16;
  static Player* singleton;
  Player();
};

#endif /* MAIN_AUDIO_PLAYER_H_ */
