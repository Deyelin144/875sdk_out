#ifndef SDK_EXCEPTION_RUNTIME_H
#define SDK_EXCEPTION_RUNTIME_H

void exception_wrapper_set_ex_status(unsigned char status);
unsigned char exception_wrapper_get_ex_status(void);
int exception_wrapper_save_log_append(char *buffer, int len, unsigned char finish);
void sdk_exception_reset_capture(void);
int sdk_exception_get_capture_buffer(char **buffer, int *len);

#endif
