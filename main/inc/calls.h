// NVS
extern void utils_nvs_init(void);
extern esp_err_t utils_nvs_get_str(const char *  , char * , size_t );
extern void utils_nvs_erase_all(void);
extern esp_err_t utils_nvs_get_int(const char * , int16_t *);
extern void utils_nvs_erase_key(const char *);
extern void utils_nvs_set_int(const char *  , int16_t);
extern void load_settings_nvs(void);
extern void utils_nvs_set_str(const char * , const char *);
//Analyse Packet UART
extern void process_uart_packet(const char *);

//uart
extern void uart_write_string(const char *);
extern void uart_write_string_ln(const char *);
extern void uart_write_number(uint8_t);

// main
extern bool extractSubstring(const char* , char* );

// tcp
extern void tcp_ip_client_send_str(const char *);
extern void tcp_ip_client_send_str(const char *);

// fota
extern void http_fota(void);

extern void set_led_state(Led_State_t);

