/*
 * SOCK_IN: User input functions moved from pctcp.c;
 *
 *  Functions used in these macros:
 *    sock_wait_established()
 *    sock_wait_input()
 *    sock_wait_closed();
 */

#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "language.h"
#include "misc.h"
#include "pctcp.h"

/*
 * ip user level timer stuff
 *   void ip_timer_init (void *s, int delayseconds)
 *   int  ip_timer_expired (void *s)
 *  - 0 if not expired
 */
void ip_timer_init (sock_type *s, int seconds)
{
    switch (s->u.ip_type) {
    case UDP_PROTO:
    case TCP_PROTO:
        if (seconds) {
            s->u.usertimer = set_timeout (1000 * seconds);
        } else {
            s->u.usertimer = 0;
        }
        break;
    }
}

int ip_timer_expired (const sock_type *s)
{
    switch (s->u.ip_type) {
    case UDP_PROTO:
    case TCP_PROTO:
        return chk_timeout (s->u.usertimer);
    }
    return (0);
}

/*
 * _ip_delay0 called by macro sock_wait_established()
 * _ip_delay1 called by macro sock_wait_input()
 * _ip_delay2 called by macro sock_wait_closed();
 *
 */
int _ip_delay0 (sock_type *s, int timeoutseconds, UserHandler fn, int *statusptr)
{
  int status = -1;

  ip_timer_init (s, timeoutseconds);
  for ( ;; )
  {
#if !defined(USE_UDP_ONLY)
    if (s->u.ip_type == TCP_PROTO)
    {
      if (tcp_established(&s->tcp))
      {
        status = 0;
        break;
      }
    }
#endif

    kbhit();        /* permit ^C */

    if (!tcp_tick(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Host refused connection");
      status = -1;       /* get an early reset */
      break;
    }
    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Open timed out");
      sock_close (s);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
      (*s->tcp.usr_yield)();

    if (s->u.ip_type == UDP_PROTO)
    {
      status = 0;
      break;
    }
  }

  if (statusptr)
     *statusptr = status;
  return (status);
}

int _ip_delay1 (sock_type *s, int timeoutseconds, UserHandler fn, int *statusptr)
{
  int status = -1;

  ip_timer_init (s, timeoutseconds);

#if !defined(USE_UDP_ONLY)
  sock_flush (s);    /* new enhancement */
#endif

  for ( ;; )
  {
    if (sock_dataready(s))
    {
      status = 0;
      break;
    }
    kbhit();         /* permit ^C */

    if (!tcp_tick(s))
    {
      status = 1;
      break;
    }
    if ((s->tcp.locflags & LF_GOT_FIN) && !sock_dataready(s))
    {
      status = 1;
      break;
    }

    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Connection timed out");
      sock_close (s);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
      (*s->tcp.usr_yield)();
  }

  if (statusptr)
     *statusptr = status;
  return (status);
}

int _ip_delay2 (sock_type *s, int timeoutseconds, UserHandler fn, int *statusptr)
{
  int status = -1;

  if (s->u.ip_type != TCP_PROTO)
  {
    if (statusptr)
       *statusptr = 1;
    return (1);
  }

#if !defined(USE_UDP_ONLY)
  ip_timer_init (s, timeoutseconds);

  for ( ;; )
  {
    /* in this situation we know user is not planning to read rdata
     */
    s->tcp.rdatalen = 0;
    kbhit();              /* permit ^C */
    if (!tcp_tick(s))
    {
      status = 1;
      break;
    }
    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Connection timed out");
      sock_abort (s);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
      (*s->tcp.usr_yield)();
  }

  if (statusptr)
     *statusptr = status;

#else
  ARGSUSED (fn);
  ARGSUSED (timeoutseconds);
#endif
  return (status);
}

int sock_timeout (sock_type *s, int sec)
{
  if (s->u.ip_type != TCP_PROTO)
     return (1);

#if !defined(USE_UDP_ONLY)
  if (s->tcp.state != tcp_StateESTAB)
     return (2);

  s->tcp.timeout = set_timeout (1000 * sec);
#else
  ARGSUSED (sec);
#endif
  return (0);
}

int sock_established (sock_type *s)
{
  switch (s->u.ip_type)
  {
    case UDP_PROTO:
         return (1);

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         return (s->tcp.state == tcp_StateESTAB ||
                 s->tcp.state == tcp_StateESTCL ||
                 s->tcp.state == tcp_StateCLOSWT);
#endif
    default:
         return (0);
  }
}


