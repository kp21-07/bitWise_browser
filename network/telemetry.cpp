#include "../includes/telemetry.hpp"
#include <chrono>

using namespace chrono;

int getTime() {
  int milli = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  return milli;
}

string getCurrentUrl() { return currentUrl; }
