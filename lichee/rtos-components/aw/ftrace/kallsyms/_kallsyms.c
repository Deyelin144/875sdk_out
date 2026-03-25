#include "_kallsyms.h"

#if DYNAMIC_FTRACE_KALLSYMS

ADD_ATTRIBUTE \
static addr_t kallsyms_sym_address(int idx)
{
	return kallsyms_addresses[idx];
}

static unsigned int get_symbol_seq(int index)
{
	uint32_t i, seq = 0;

	for (i = 0; i < 3; i++)
		seq = (seq << 8) | kallsyms_seqs_of_names[3 * index + i];

	return seq;
}

ADD_ATTRIBUTE \
static unsigned long get_symbol_pos(unsigned long addr,
				    unsigned long *symbolsize,
				    unsigned long *offset)
{
	unsigned long symbol_start = 0, symbol_end = 0;
	unsigned long i, low, high, mid;

	/* Do a binary search on the sorted kallsyms_addresses array. */
	low = 0;
	high = kallsyms_num_syms;

	while (high - low > 1) {
		mid = low + (high - low) / 2;
		if (kallsyms_sym_address(mid) <= addr)
			low = mid;
		else
			high = mid;
	}

	/*
	 * Search for the first aliased symbol. Aliased
	 * symbols are symbols with the same address.
	 */
	while (low && kallsyms_sym_address(low-1) == kallsyms_sym_address(low))
		--low;

	symbol_start = kallsyms_sym_address(low);

	/* Search for next non-aliased symbol. */
	for (i = low + 1; i < kallsyms_num_syms; i++) {
		if (kallsyms_sym_address(i) > symbol_start) {
			symbol_end = kallsyms_sym_address(i);
			break;
		}
	}

#if 0
	/* If we found no next symbol, we use the end of the section. */
	if (!symbol_end) {
		if (is_kernel_inittext(addr))
			symbol_end = (unsigned long)_einittext;
		else if (IS_ENABLED(CONFIG_KALLSYMS_ALL))
			symbol_end = (unsigned long)_end;
		else
			symbol_end = (unsigned long)_etext;
	}
#endif
	if (symbolsize)
		*symbolsize = symbol_end - symbol_start;
	if (offset)
		*offset = addr - symbol_start;

	return low;
}

/*
 * Find the offset on the compressed stream given and index in the
 * kallsyms array.
 */
ADD_ATTRIBUTE \
static unsigned int get_symbol_offset(unsigned long pos)
{
	const unsigned char *name = NULL;
	int i, len;

	/*
	 * Use the closest marker we have. We have markers every 256 positions,
	 * so that should be close enough.
	 */
	name = &kallsyms_names[kallsyms_markers[pos >> 8]];

	/*
	 * Sequentially scan all the symbols up to the point we're searching
	 * for. Every symbol is stored in a [<len>][<len> bytes of data] format,
	 * so we just need to add the len to the current pointer for every
	 * symbol we wish to skip.
	 */
	for (i = 0; i < (pos & 0xFF); i++) {
		len = *name;

		/*
		 * If MSB is 1, it is a "big" symbol, so we need to look into
		 * the next byte (and skip it, too).
		 */
		if ((len & 0x80) != 0) {
			len = ((len & 0x7F) | (name[1] << 7)) + 1;
		}

		name = name + len + 1;
	}

	return name - kallsyms_names;
}
/*
 * Expand a compressed symbol data into the resulting uncompressed string,
 * if uncompressed string is too long (>= maxlen), it will be truncated,
 * given the offset to where the symbol is in the compressed stream.
 */
ADD_ATTRIBUTE \
static unsigned int kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen)
{
	int len, skipped_first = 0;
	const char *tptr;
	const unsigned char *data;

	/* Get the compressed symbol length from the first symbol byte. */
	data = &kallsyms_names[off];
	len = *data;
	data++;
	off++;

	/* If MSB is 1, it is a "big" symbol, so needs an additional byte. */
	if ((len & 0x80) != 0) {
		len = (len & 0x7F) | (*data << 7);
		data++;
		off++;
	}

	/*
	 * Update the offset to return the offset for the next symbol on
	 * the compressed stream.
	 */
	off += len;

	/*
	 * For every byte on the compressed symbol data, copy the table
	 * entry for that byte.
	 */
	while (len) {
		tptr = &kallsyms_token_table[kallsyms_token_index[*data]];
		data++;
		len--;

		while (*tptr) {
			if (skipped_first) {
				if (maxlen <= 1)
					goto tail;
				*result = *tptr;
				result++;
				maxlen--;
			} else
				skipped_first = 1;
			tptr++;
		}
	}

tail:
	if (maxlen)
		*result = '\0';

	/* Return to offset to the next symbol. */
	return off;
}

ADD_ATTRIBUTE \
static int compare_symbol_name(const char *name, char *namebuf)
{
	/* The kallsyms_seqs_of_names is sorted based on names after
	 * cleanup_symbol_name() (see scripts/kallsyms.c) if clang lto is enabled.
	 * To ensure correct bisection in kallsyms_lookup_names(), do
	 * cleanup_symbol_name(namebuf) before comparing name and namebuf.
	 */
//	cleanup_symbol_name(namebuf);
	return strcmp(name, namebuf);
}

#undef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define KSYM_NAME_LEN 512

ADD_ATTRIBUTE \
static int kallsyms_lookup_names(const char *name,
				 unsigned int *start,
				 unsigned int *end)
{
	int ret;
	int low, mid, high;
	unsigned int seq, off;
	char namebuf[KSYM_NAME_LEN];

	low = 0;
	high = kallsyms_num_syms - 1;

	while (low <= high) {
		mid = low + (high - low) / 2;
		seq = get_symbol_seq(mid);
		off = get_symbol_offset(seq);
		kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));
		ret = compare_symbol_name(name, namebuf);
		if (ret > 0)
			low = mid + 1;
		else if (ret < 0)
			high = mid - 1;
		else
			break;
	}

	if (low > high)
		return -1;

	low = mid;
	while (low) {
		seq = get_symbol_seq(low - 1);
		off = get_symbol_offset(seq);
		kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));
		if (compare_symbol_name(name, namebuf))
			break;
		low--;
	}
	*start = low;

	if (end) {
		high = mid;
		while (high < kallsyms_num_syms - 1) {
			seq = get_symbol_seq(high + 1);
			off = get_symbol_offset(seq);
			kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));
			if (compare_symbol_name(name, namebuf))
				break;
			high++;
		}
		*end = high;
	}

	return 0;
}
/* Lookup the address for this symbol. Returns 0 if not found. */
ADD_ATTRIBUTE \
unsigned long kallsyms_lookup_name(const char *name)
{
	int ret;
	unsigned int i;

	/* Skip the search for empty string. */
	if (!*name)
		return 0;

	ret = kallsyms_lookup_names(name, &i, NULL);
	if (!ret)
		return kallsyms_sym_address(get_symbol_seq(i));

	return ret;
}

ADD_ATTRIBUTE \
int kallsyms_addr2name(void *addr, char *buf)
{
	uint32_t func_name_len = 0;
	char str1[64] = {0};
	const char *str2 = NULL;
	short kallsyms_table_offset_bytes;
	int token_index;
	int func_sn;
	int func_name_start;
	int last_func_name_start;
	int current_func_name_start;
	addr_t func_addr;
	unsigned long symbolsize;
	unsigned long offset;
	unsigned long kallsyms_markers_index;

	func_addr = (addr_t)addr;
	func_sn = get_symbol_pos(func_addr,&symbolsize, &offset);

	kallsyms_markers_index = func_sn >> 8;
	func_sn = func_sn & 0xff;
	last_func_name_start = kallsyms_markers[kallsyms_markers_index];

	current_func_name_start = 0;
	func_sn = func_sn + 1;

	for (int i = 0; i < func_sn; i ++) {
		func_name_len = kallsyms_names[last_func_name_start];
		current_func_name_start = last_func_name_start + 1;
		func_name_start = last_func_name_start + func_name_len + 1;
		last_func_name_start = func_name_start;
	}

	for (int i = current_func_name_start; i < current_func_name_start + func_name_len; i++) {
		token_index = kallsyms_names[i];
		kallsyms_table_offset_bytes = kallsyms_token_index[token_index];
		str2 = &kallsyms_token_table[kallsyms_table_offset_bytes];
		strcat(str1, str2);
	}
	memcpy(buf, str1, strlen(str1) + 1);

	return 0;
}
#endif /* DYNAMIC_FTRACE_KALLSYMS */
