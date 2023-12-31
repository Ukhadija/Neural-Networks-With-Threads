#include<iostream>
#include<atomic>
using namespace std;

extern const int input_neurons;
extern const int hidden_layers_neurons;

struct input_attr
{
    public:
    float weights[2];
    atomic_int ith_value;
    float input_weights[8];

};

struct middle_attr
{
    public:
    float input[8];
    float weights[8];
    atomic_int ith_value;
    atomic_int p_value;
};
