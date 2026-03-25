/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "table.h"

/* debug */
#define MODULE_NAME "[TABLE]"
#include "debug.h"

#if DYNAMIC_FTRACE
ADD_ATTRIBUTE \
int table_init(table_t *_table)
{
	memset(_table->table, 0, TABLE_SIZE);
	_table->table_index = 0;

	return 0;
}

ADD_ATTRIBUTE \
int table_add(table_t *_table, void *_addr)
{
	addr_t addr = (addr_t)_addr;
	_32bit *table_index = &_table->table_index;
	addr_t *table = _table->table;

	if (*table_index >= TABLE_SIZE) {
		ERR(("TABLE IS FULL. SET FAILED!\n"));
		return -1;
	}
	for (int i = 0; i < *table_index; i ++) {
		if (table[i] == addr) {
			ERR(("%x ALREADY IN TABLE\n", addr));
			return 0;
		}
	}
	table[*table_index] = addr;
	*table_index += 1;

	return 0;
}

ADD_ATTRIBUTE \
int table_clear(table_t *_table, void *_addr)
{
	addr_t addr = (addr_t)_addr;
	_32bit *table_index = &_table->table_index;
	addr_t *table = _table->table;

	if (*table_index == 0) {
		ERR(("TABLE IS EMPTY!\n"));
		return -1;
	}
	for (int i = 0; i < *table_index; i ++) {
		if (table[i] == addr) {
			table[i] = table[*table_index - 1];
			table[*table_index - 1] = 0;
			*table_index -= 1;
		}
	}

	return 0;
}

ADD_ATTRIBUTE \
int table_dump(table_t *_table)
{
	addr_t addr;
	_32bit *table_index = &_table->table_index;
	addr_t *table = _table->table;

	if (*table_index == 0) {
		ERR(("TABLE IS EMPTY!\n"));
		return -1;
	}
	for (int i = 0; i < *table_index; i ++) {
		addr = table[i];
		INF(("[%d] %x\n", i, addr));
	}

	return 0;
}

ADD_ATTRIBUTE \
int table_flush(table_t *_table)
{
	_32bit *table_index = &_table->table_index;
	addr_t *table = _table->table;

	for (int i = 0; i < *table_index; i ++) {
		table[i] = 0;
	}
	*table_index = 0;

	return 0;
}

ADD_ATTRIBUTE \
int table_find(table_t *_table, void *_addr)
{
	addr_t addr = (addr_t)_addr;
	_32bit *table_index = &_table->table_index;
	addr_t *table = _table->table;

	if (*table_index == 0) {
		return 0;
	}
	for (int i = 0; i < *table_index; i ++) {
		if (table[i] == addr) {
			return 1;
		}
	}

	return -1;
}
#endif /* DYNAMIC_FTRACE */
