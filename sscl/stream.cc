/* SSCL - Symbion Simple Class Library
 * Copyright (C) 2001 Szilard Hajba
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <errno.h>

#include <sscl/error.h>
#include <sscl/stream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace SSCL {

NullStream::NullStream(const int fd)
{
    stream_init_fd(&cs, fd, 0);
}

NullStream::NullStream(const char *file, const int flags)
{
    stream_init_file(&cs, file, flags, 0);
}

InStream::InStream(const int fd, const int len): NullStream(fd)
{
    stream_init_fd(&cs, fd, len);
}

InStream::InStream(const char *file, const int flags, const int len): NullStream(file, flags)
{
    stream_init_file(&cs, file, flags, len);
}

OutStream::OutStream(int fd, const int len): NullStream(fd)
{
    stream_init_fd(&cs, fd, len);
}

Stream::Stream(int fd, const int ilen, const int olen):
	NullStream(fd), InStream(fd, ilen), OutStream(fd) {}

} /* namespace SSCL */
