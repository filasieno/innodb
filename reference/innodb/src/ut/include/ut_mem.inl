#include "ut_byte.hpp"
#include "mach_data.hpp"

IB_INLINE void* ut_memcpy(void* dest, const void* sour, ulint n)
{
	return memcpy(dest, sour, n);
}

IB_INLINE void* ut_memmove(void* dest, const void* sour, ulint n)
{
	return memmove(dest, sour, n);
}

IB_INLINE int ut_memcmp(const void* str1, const void* str2, ulint n)
{
	return memcmp(str1, str2, n);
}

IB_INLINE char* ut_strcpy(char* dest, const char* sour)
{
	return strcpy(dest, sour);
}

IB_INLINE ulint ut_strlen(const char* str)
{
	return strlen(str);
}

IB_INLINE int ut_strcmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
}

IB_INLINE ulint ut_strlenq(const char*	str, char q)
{
	for (ulint len = 0; *str; len++, str++) {
		if (*str == q) {
			len++;
		}
	}
	return len;
}

IB_INLINE ulint ut_raw_to_hex(const void* raw, ulint raw_size, char* hex, ulint hex_size)	
{
	#ifdef WORDS_BIGENDIAN
		#define MK_UINT16(a, b) (((ib_uint16_t) (a)) << 8 | (ib_uint16_t) (b))
		#define UINT16_GET_A(u)	((unsigned char) ((u) >> 8))
		#define UINT16_GET_B(u)	((unsigned char) ((u) & 0xFF))
	#else
		#define MK_UINT16(a, b) (((ib_uint16_t) (b)) << 8 | (ib_uint16_t) (a))
		#define UINT16_GET_A(u)	((unsigned char) ((u) & 0xFF))
		#define UINT16_GET_B(u)	((unsigned char) ((u) >> 8))
	#endif

	#define MK_ALL_UINT16_WITH_A(a)\
		MK_UINT16(a, '0'),\
		MK_UINT16(a, '1'),\
		MK_UINT16(a, '2'),\
		MK_UINT16(a, '3'),\
		MK_UINT16(a, '4'),\
		MK_UINT16(a, '5'),\
		MK_UINT16(a, '6'),\
		MK_UINT16(a, '7'),\
		MK_UINT16(a, '8'),\
		MK_UINT16(a, '9'),\
		MK_UINT16(a, 'A'),\
		MK_UINT16(a, 'B'),\
		MK_UINT16(a, 'C'),\
		MK_UINT16(a, 'D'),\
		MK_UINT16(a, 'E'),\
		MK_UINT16(a, 'F')

	static const ib_uint16_t hex_map[256] = {
		MK_ALL_UINT16_WITH_A('0'),
		MK_ALL_UINT16_WITH_A('1'),
		MK_ALL_UINT16_WITH_A('2'),
		MK_ALL_UINT16_WITH_A('3'),
		MK_ALL_UINT16_WITH_A('4'),
		MK_ALL_UINT16_WITH_A('5'),
		MK_ALL_UINT16_WITH_A('6'),
		MK_ALL_UINT16_WITH_A('7'),
		MK_ALL_UINT16_WITH_A('8'),
		MK_ALL_UINT16_WITH_A('9'),
		MK_ALL_UINT16_WITH_A('A'),
		MK_ALL_UINT16_WITH_A('B'),
		MK_ALL_UINT16_WITH_A('C'),
		MK_ALL_UINT16_WITH_A('D'),
		MK_ALL_UINT16_WITH_A('E'),
		MK_ALL_UINT16_WITH_A('F')
	};

	ulint read_bytes;
	ulint write_bytes;
	const unsigned char* rawc = (const unsigned char*) raw;
	if (hex_size == 0) {
		return 0;
	}
	if (hex_size <= 2 * raw_size) {
		read_bytes = hex_size / 2;
		write_bytes = hex_size;
	} else {
		read_bytes = raw_size;
		write_bytes = 2 * raw_size + 1;
	}
	#define LOOP_READ_BYTES(ASSIGN)\
		{\
			for (ulint i = 0; i < read_bytes; i++) {\
				ASSIGN;\
				hex += 2;\
				rawc++;\
			}\
		}
	if (ut_align_offset(hex, 2) == 0) {
		LOOP_READ_BYTES(*(ib_uint16_t*) hex = hex_map[*rawc]);
	} else {
		LOOP_READ_BYTES(
			*hex = UINT16_GET_A(hex_map[*rawc]);
			*(hex + 1) = UINT16_GET_B(hex_map[*rawc])
		);
	}
	if (hex_size <= 2 * raw_size && hex_size % 2 == 0) {
		hex--;
	}
	*hex = '\0';
	return write_bytes;
}

IB_INLINE ulint ut_str_sql_format(const char* str, ulint str_len, char* buf, ulint buf_size)					
{
	ulint buf_i = 0;
	switch (buf_size) {
	case 3:
		if (str_len == 0) {
			buf[buf_i] = '\'';
			buf_i++;
			buf[buf_i] = '\'';
			buf_i++;
		}
		/* fallthrough */
	case 2:
	case 1:
		buf[buf_i] = '\0';
		buf_i++;
		/* fallthrough */
	case 0:
		return buf_i;
	}
	buf[0] = '\'';
	buf_i = 1;
	for (ulint str_i = 0; str_i < str_len; str_i++) {
		if (buf_size - buf_i == 2) {
			break;
		}
		char ch = str[str_i];
		switch (ch) {
		case '\0':
			if (IB_UNLIKELY(buf_size - buf_i < 4)) {
				goto func_exit;
			}
			buf[buf_i] = '\\';
			buf_i++;
			buf[buf_i] = '0';
			buf_i++;
			break;
		case '\'':
		case '\\':
			if (IB_UNLIKELY(buf_size - buf_i < 4)) {
				goto func_exit;
			}
			buf[buf_i] = ch;
			buf_i++;
			/* fallthrough */
		default:
			buf[buf_i] = ch;
			buf_i++;
		}
	}
func_exit:
	buf[buf_i] = '\'';
	buf_i++;
	buf[buf_i] = '\0';
	buf_i++;
	return buf_i ;
}
