/*  $Id$

    Part of SWI-Prolog

    Author:        Jan Wielemaker
    E-mail:        J.Wielemaker@uva.nl
    WWW:           http://www.swi-prolog.org
    Copyright (C): 2008, University of Amsterdam

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "pl-incl.h"

#define setHandle(h, w)		(*valTermRef(h) = (w))
#define valHandleP(h)		valTermRef(h)

static inline word
valHandle(term_t r)
{ Word p = valTermRef(r);

  deRef(p);
  return *p;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This module defines extensions to pl-fli.c that are used internally, but
not exported to the SWI-Prolog user. Most   of them are too specific for
the public interface.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


		 /*******************************
		 *     CHARACTER GET/UNIFY	*
		 *******************************/

/** PL_get_char(term_t c, int *p, int eof)

Get a character code from a term and   store  in over p. Returns TRUE if
successful. On failure it returns a  type   error.  If  eof is TRUE, the
integer -1 or the atom end_of_file can used to specify and EOF character
code.
*/

int
PL_get_char(term_t c, int *p, int eof)
{ GET_LD
  int chr;
  atom_t name;
  PL_chars_t text;

  if ( PL_get_integer(c, &chr) )
  { if ( chr >= 0 )
    { *p = chr;
      return TRUE;
    }
    if ( eof && chr == -1 )
    { *p = chr;
      return TRUE;
    }
  } else if ( PL_get_text(c, &text, CVT_ATOM|CVT_STRING|CVT_LIST) &&
	      text.length == 1 )
  { *p = text.encoding == ENC_ISO_LATIN_1 ? text.text.t[0]&0xff
					  : text.text.w[0];
    return TRUE;
  } else if ( eof && PL_get_atom(c, &name) && name == ATOM_end_of_file )
  { *p = -1;
    return TRUE;
  }

  return PL_error(NULL, 0, NULL, ERR_TYPE, ATOM_character, c);
}


/** PL_unify_char(term_t chr, int c, int how)

Unify a character.  Try to be as flexible as possible, only binding a
variable `chr' to a code or one-char-atom.  E.g., this succeeds:

    PL_unify_char('a', 97, PL_CODE)
*/

int
PL_unify_char(term_t chr, int c, int how)
{ GET_LD
  int c2 = -1;

  if ( PL_is_variable(chr) )
  { switch(how)
    { case PL_CHAR:
      { atom_t a = (c == -1 ? ATOM_end_of_file : codeToAtom(c));

	return PL_unify_atom(chr, a);
      }
      case PL_CODE:
      case PL_BYTE:
      default:
	return PL_unify_integer(chr, c);
    }
  } else if ( PL_get_char(chr, &c2, TRUE) )
    return c == c2;

  return FALSE;
}
