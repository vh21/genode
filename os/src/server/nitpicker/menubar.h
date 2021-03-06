/*
 * \brief  Nitpicker menubar interface
 * \author Norman Feske
 * \date   2006-08-22
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _MENUBAR_H_
#define _MENUBAR_H_

#include "view.h"
#include "draw_label.h"
#include "mode.h"


struct Menubar
{
	virtual ~Menubar() { }

	/**
	 * Set state that is displayed in the trusted menubar
	 */
	virtual void state(Mode const &mode, char const *session_label,
	                   char const *view_title, Genode::Color session_color) = 0;
};

#endif
