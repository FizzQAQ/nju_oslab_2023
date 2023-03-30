#include "philosopher.h"

// TODO: define some sem if you need
uint32_t forks[5],people;
void init() {
 for(int i=0;i<5;i++){
  forks[i]=sem_open(1);
 }
 people=sem_open(4);
}

void philosopher(int id) {
  // implement philosopher, remember to call `eat` and `think`
  while (1) {
    think(id);
    P(people);
    P(forks[id]);
    P(forks[(id+1)%5]);
    eat(id);
    V(forks[(id+1)%5]);
    V(forks[id]);
    V(people);
  }
}
