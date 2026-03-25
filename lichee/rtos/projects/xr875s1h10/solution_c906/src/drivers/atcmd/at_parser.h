#ifndef AT_PARSER_H_
#define AT_PARSER_H_

#include <stdint.h>

int at_parser_feed(uint8_t byte);
const char* at_parser_get_cmd(void);

#endif /* AT_PARSER_H_ */

