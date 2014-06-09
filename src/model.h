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
} __attribute__((__packed__)) Model;

void model_init(Model *model);
void model_incr(Model *model, int changed, double x);
double model_get_field(Model *model, int field);
double model_get_double(Model *model, int field);
