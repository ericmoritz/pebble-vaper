#include <pebble.h>
#define WATTS 1
#define AMPS  2
#define VOLTS 4
#define OHMS  8

typedef struct Model {
  int locked;
  double r;
  double v;
  double i;
  double p;
} Model;

void model_init(Model *model);
void model_incr(Model *model, int changed, double x);

