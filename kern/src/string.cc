/*
 * String Functions
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#include "types.h"

extern "C" void *memmove(void *d, void const *s, size_t n) {
    if (d == s || n == 0)
        return d;

    unsigned char *dst = static_cast<unsigned char *>(d);
    unsigned char const *src = static_cast<unsigned char const *>(s);

    if (dst < src) {
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    } else {
        for (size_t i = n; i > 0; --i)
            dst[i - 1] = src[i - 1];
    }

    return d;
}
