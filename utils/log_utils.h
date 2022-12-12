#ifndef HSV_MAPPER_LOG_UTILS_H
#define HSV_MAPPER_LOG_UTILS_H

#include <string>

std::string getTimestampStr();

long long getTimeMs();

void glErrCallback(int, const char *);

#endif //HSV_MAPPER_LOG_UTILS_H
