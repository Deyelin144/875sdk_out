/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Aligned Memory Allocator -
 *
 *  Copyright(C) 2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id$
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "utils/mem_align.h"

/*****************************************************************************
 * xvid_malloc
 *
 * This function allocates 'size' bytes (usable by the user) on the heap and
 * takes care of the requested 'alignment'.
 * In order to align the allocated memory block, the xvid_malloc allocates
 * 'size' bytes + 'alignment' bytes. So try to keep alignment very small
 * when allocating small pieces of memory.
 *
 * NB : a block allocated by xvid_malloc _must_ be freed with xvid_free
 *      (the libc free will return an error)
 *
 * Returned value : - NULL on error
 *                  - Pointer to the allocated aligned block
 *
 ****************************************************************************/

void hal_free_coherent_align(void *addr);
void *hal_malloc_coherent_align(size_t size, int align);

// extern void *mp4_port_custom_malloc_coherent_align(size_t size, int align);
// extern void mp4_port_custom_free_coherent_align(void *addr);

void *xvid_malloc(size_t size, uint8_t alignment)
{
    return hal_malloc_coherent_align(size, alignment);
    // return mp4_port_custom_malloc_coherent_align(size, alignment);
}

/*****************************************************************************
 * xvid_free
 *
 * Free a previously 'xvid_malloc' allocated block. Does not free NULL
 * references.
 *
 * Returned value : None.
 *
 ****************************************************************************/

void xvid_free(void *mem_ptr)
{
    hal_free_coherent_align(mem_ptr);
    // mp4_port_custom_free_coherent_align(mem_ptr);
}
