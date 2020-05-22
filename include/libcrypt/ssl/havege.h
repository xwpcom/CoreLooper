#ifdef _SUPPORT_SSL
/**
 * \file havege.h
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
#ifndef POLARSSL_HAVEGE_H
#define POLARSSL_HAVEGE_H

#define COLLECT_SIZE 1024

/**
 * \brief          HAVEGE state structure
 */
typedef struct
{
    int PT1, PT2, offset[2];
    int pool[COLLECT_SIZE];
    int WALK[8192];
}
havege_state;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          HAVEGE initialization
 *
 * \param hs       HAVEGE state to be initialized
 */
void havege_init_EX( havege_state *hs );

/**
 * \brief          HAVEGE rand function
 *
 * \param p_rng    A HAVEGE state
 *
 * \return         A random int
 */
int havege_rand_EX( void *p_rng );

#ifdef __cplusplus
}
#endif

#endif /* havege.h */
#endif//#ifdef _SUPPORT_SSL
