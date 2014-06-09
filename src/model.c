#include <pebble.h>
#include <math.h>
#include "model.h"

void model_init(Model *model) {
  model->locked = OHMS;
  model->r = 1.00;
  model->v = 4.10;
  model->i = 4.10;
  model->p = 16.81;
}


double cap(double x) {
  return x;
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
      model->p = cap(model->p + x);
      break;
    case AMPS:
      model->i = cap(model->i + x);
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

double model_get_field(Model *model, int field) {
  switch(field) {
  case WATTS:
    return model->p;
  case AMPS:
    return model->i;
  case VOLTS:
    return model->v;
  default:
    return model->r;
  }
}
double model_get_double(Model *model, int field) {
  return model_get_field(model, field);
}
