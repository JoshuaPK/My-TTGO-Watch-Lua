/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define __LUA_VERSION__ 5.4.3

#ifndef _LUA_RT_APP_MAIN_H
    #define _LUA_RT_APP_MAIN_H

    #include <TTGO.h>
    #include "lua-__LUA_VERSION__-src/lua.h"

    // Declare the Lua contexts & stuff here
    // Good example on how to do this:
    // https://github.com/jakub-vesely/HuGo
	

    

    void lua_rt_app_main_setup( uint32_t tile_num );
    bool InitLua( void );
    bool DoLua( void );
    void CloseLua(void );

#endif // _LUA_RT_APP_MAIN_H
