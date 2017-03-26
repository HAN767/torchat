struct data_wrapper *convert_string_to_datastruct(const char *jsonCh);
char *convert_datastruct_to_char(const struct data_wrapper *data);
void log_info(char *json);
void log_err(char *json);
extern char *HOSTNAME;
bool parse_connection(const int sock,struct data_wrapper **retData,char **retJson,int64_t deadline);
void free_data_wrapper(struct data_wrapper *data);
void announce_exit(struct data_wrapper *data,int sock);
void relay_msg(const int clientSock,struct data_wrapper *data,int64_t deadline);
void store_msg(struct data_wrapper *data);
void client_update(struct data_wrapper *data,int sock,int64_t deadline);
void send_hostname_to_client(struct data_wrapper *data,int sock,char *hostname,int64_t deadline);
void send_peer_list_to_client(struct data_wrapper *data,int sock,int64_t deadline);
