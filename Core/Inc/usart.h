
#ifndef INC_USART_H_
#define INC_USART_H_

void USART_INIT();
void usart_write(const char* s);
char usart_get_char(void);
void usart_put_char(char c);
int usart_readline(char *buf, int max);

#endif /* INC_USART_H_ */
