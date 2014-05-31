#include <pebble.h>
#include "model.h"

#define SQRT_MAGIC_F 0x5f3759df 

double up = 0.1f;
double down = -0.1f;

void model_init(Model *model) {
  model->locked = OHMS;
  model->r = 2.5f;
  model->v = 3.7f;
  model->i = 1.4f;
  model->p = 4.9f;
}


double cap(double x) {
  return x;
}

float my_sqrt(const float x)
{
  const float xhalf = 0.5f*x;
 
  union // get bits for floating value
  {
    float x;
    int i;
  } u;
  u.x = x;
  u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
  return x*u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy 
}  

void model_update(Model *model, int changed) {
  switch(model->locked | changed) {
  case VOLTS | OHMS:
  case VOLTS:
  case OHMS:
    model->i = model->v / model->r;
    model->p = model->v * model->i;
    break;
  }
}

void model_incr(Model *model, int changed, double x) {
  switch(changed) {
    case WATTS:
      //model->p = cap(model->p + x); // re-enable when watt/amps are supported
      break;
    case AMPS:
      //model->i = cap(model->i + x);
      break;
    case VOLTS:
      model->v = cap(model->v + x);
      break;
    case OHMS:
      model->r = cap(model->r + x);
      break;
  }
  model_update(model, changed);
}


