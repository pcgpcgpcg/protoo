#include "utils.h"
#include <random>

using std::default_random_engine;
using std::uniform_real_distribution;

int generateRandomNumber() {
	default_random_engine e;
	uniform_real_distribution<double> u(0, 1);
	return int(u(e) * 10000000);
}