
/*
 * Copyright (C) 2014-2015 Galois, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License** as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.

 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

#include <backtrace.h>

#include "serial.h"

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
	volatile unsigned long ul = 1;
	uint32_t cpu_sr;

	int cur_cpu_id(void);
	taskENTER_CRITICAL(cpu_sr);
	{
		backtrace(NULL, NULL, 0, 0, printf);
		SMP_DBG("cpu%d: pcFile %s, uLine %d, task handle %p.\n", cur_cpu_id(), \
			pcFile, ulLine, xTaskGetCurrentTaskHandle());
		while(ul);
	}
	taskEXIT_CRITICAL(cpu_sr);
}

