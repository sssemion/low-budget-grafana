#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <ctime>
#include <vector>

std::string formatTimestamp(std::time_t timestamp);

int valueTickFormatter(double value, char *buff, int size, void *user_data);

#endif // APP_UTILS_H
