#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <ctime>
#include <vector>

int timeTickFormatter(double value, char *buff, int size, void *user_data);

std::string formatTimestamp(std::time_t timestamp);

#endif // APP_UTILS_H
