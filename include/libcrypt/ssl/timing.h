#ifdef _SUPPORT_SSL
/**
 * \file timing.h
 *
 *  Copyright (C) 2006-2009, Paul Bakker <polarssl_maintainer at polarssl.org>
 *  All rights reserved.
 *
 *  Joined copyright on original XySSL code with: Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_TIMING_H
#define POLARSSL_TIMING_H

/**
 * \brief          timer structure
 */
struct hr_time_EX
{
    unsigned char opaque[32];
};

#ifdef __cplusplus
extern "C" {
#endif

//extern int alarmed;

/**
 * \brief          Return the CPU cycle counter value
 */
unsigned long hardclock_EX( void );

/**
 * \brief          Return the elapsed time in milliseconds
 *
 * \param val      points to a timer structure
 * \param reset    if set to 1, the timer is restarted
 */
unsigned long get_timer_EX( struct hr_time_EX *val, int reset );

/**
 * \brief          Setup an alarm clock
 *
 * \param seconds  delay before the "alarmed" flag is set
 */
void set_alarm_EX( int seconds );

/**
 * \brief          Sleep for a certain amount of time
 *
 * \param          Delay in milliseconds
 */
void m_sleep_EX( int milliseconds );

#ifdef __cplusplus
}
#endif

#endif /* timing.h */
#endif//#ifdef _SUPPORT_SSL
