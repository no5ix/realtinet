/* Copyright (C) 1997-2014 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, see
<http://www.gnu.org/licenses/>.  */


// delete some pre-compiler option

/* Event types that can be polled for.  These bits may be set in `events'
to indicate the interesting event types; they will appear in `revents'
to indicate the status of the file descriptor.  */

/* release definitions for WSAPoll(). */
#ifdef POLLIN
#undef POLLIN
#endif
#define POLLIN          0x001           /* There is data to read.  */

#ifdef POLLPRI
#undef POLLPRI
#endif
#define POLLPRI         0x002           /* There is urgent data to read.  */

#ifdef POLLOUT
#undef POLLOUT
#endif
#define POLLOUT         0x004           /* Writing now will not block.  */


/* These values are defined in XPG4.2.  */
#ifdef POLLRDNORM
#undef POLLRDNORM
#endif
# define POLLRDNORM     0x040           /* Normal data may be read.  */

#ifdef POLLRDBAND
#undef POLLRDBAND
#endif
# define POLLRDBAND     0x080           /* Priority data may be read.  */

#ifdef POLLWRNORM
#undef POLLWRNORM
#endif
# define POLLWRNORM     0x100           /* Writing now will not block.  */

#ifdef POLLWRBAND
#undef POLLWRBAND
#endif
# define POLLWRBAND     0x200           /* Priority data may be written.  */



/* These are extensions for Linux.  */
#ifdef POLLMSG
#undef POLLMSG
#endif
# define POLLMSG        0x400

#ifdef POLLREMOVE
#undef POLLREMOVE
#endif
# define POLLREMOVE     0x1000

#ifdef POLLRDHUP
#undef POLLRDHUP
#endif
# define POLLRDHUP      0x2000

/* Event types always implicitly polled for.  These bits need not be set in
`events', but they will appear in `revents' to indicate the status of
the file descriptor.  */
#ifdef POLLERR
#undef POLLERR
#endif
#define POLLERR         0x008           /* Error condition.  */

#ifdef POLLHUP
#undef POLLHUP
#endif
#define POLLHUP         0x010           /* Hung up.  */

#ifdef POLLNVAL
#undef POLLNVAL
#endif
#define POLLNVAL        0x020           /* Invalid polling request.  */

