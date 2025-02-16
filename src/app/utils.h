/**
 * @file utils.h
 * @brief Вспомогательные функции для форматирования временных меток и значений осей.
 */

/**
 * @defgroup utils Utils
 * @ingroup app
 * @brief Содержит функции для обработки временных меток и форматирования подписей осей.
 */
/** @{ */

#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <ctime>
#include <vector>

/**
 * @brief Форматирует временную метку в человекочитаемый строковый формат.
 * @param timestamp Временная метка для форматирования.
 * @return Строковое представление временной метки в читаемом формате.
 */
std::string formatTimestamp(std::time_t timestamp);

/**
 * @brief Форматирует подписи тиков на графике.
 * @details Форматирует подписи тиков на оси значений на графике в зависимости от выбранных единиц измерения.
 *          В `user_data` ожидается указатель `YAxisUnit` на выбранные единицы.
 * @param value Значение тика для форматирования.
 * @param buff Буфер для сохранения отформатированного значения.
 * @param size Размер буфера.
 * @param user_data Дополнительные пользовательские данные. Ожидается указатель `YAxisUnit`.
 * @return Количество символов, записанных в буфер.
 */
int valueTickFormatter(double value, char *buff, int size, void *user_data);

#endif // APP_UTILS_H

/** @} */
