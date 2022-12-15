#include <chrono>
#include <iomanip>
#include <ctime>
#include <string>

std::string getTimestampStr() {
  auto time = std::time(nullptr);
  auto localTime = *std::localtime(&time);
  std::ostringstream oss;
  oss << std::put_time(&localTime, "hsv_mapgen_%m%d%Y_%H%M%S");
  return oss.str();
}

long long getTimeMs() {
  auto nowTimePoint = std::chrono::high_resolution_clock::now();
  auto d = nowTimePoint.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

void glErrCallback(int error, const char *description) {
  fprintf(stderr, "Error: (%d) %s\n", error, description);
}
