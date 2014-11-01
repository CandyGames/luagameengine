// ZLIB client - used by PNG, available for other purposes

#ifdef __cplusplus
extern "C"
{
#endif

extern char *zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int   zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

extern char *zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int   zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

#ifdef __cplusplus
}
#endif
