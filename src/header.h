#ifndef HEADER_H
#define HEADER_H

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <list>
#include <stack>
#include <time.h>
#include <random>
#include <unordered_map>
#include <cmath>
using namespace std;
//******************* Parameter ******************//
const int GENERATION = 2;
const int POPULATION = 250;
const double CROSSOVER_RATE = 0.7;
const double MUTAION_RATE = 0.9;
const int K_TOURNMENT = 3; 
const double alpha = 0.5;
const double beta = 0.1;
const double delta = 0.4;
const double quota = 0.5;
const double lambda = 0.85;
const double P = 1.1;
const int HYBRID_GATE = 6000;
const double C = 100;
const double K = 4;

#endif