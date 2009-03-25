/*
 * Copyright (C) 2008 Pierre Lejeune
 *
 * This source file is intended to be compiled as a shared library
 * to be used as a hook for Paludis, the other Package Mangler.
 * It checks whether there is collisions with existing files and abort the installation if needed.
 * To use it, copy it or make a link to it into "${SHAREDIR}/paludis/merger_check_post".
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ContentsVisitorForIPFL.hh"

ContentsVisitorForIPFL::ContentsVisitorForIPFL(std::string root, ContentsList* ipfl)
{
    this->root = root;
    this->ipfl = ipfl;
}

//void ContentsVisitorForIPFL::visit(const paludis::ContentsDevEntry & d)
//{
//}

//void ContentsVisitorForIPFL::visit(const paludis::ContentsMiscEntry & d)
//{
//}

void ContentsVisitorForIPFL::visit(const paludis::ContentsFileEntry & d)
{
    this->ipfl->push_back(std::tr1::shared_ptr<paludis::ContentsEntry>(new paludis::ContentsFileEntry(d.location_key()->value())));
}

void ContentsVisitorForIPFL::visit(const paludis::ContentsDirEntry & d)
{
}

//void ContentsVisitorForIPFL::visit(const paludis::ContentsFifoEntry & d)
//{
//}

void ContentsVisitorForIPFL::visit(const paludis::ContentsOtherEntry & d)
{
}

void ContentsVisitorForIPFL::visit(const paludis::ContentsSymEntry & d)
{
    this->ipfl->push_back(std::tr1::shared_ptr<paludis::ContentsEntry>(new paludis::ContentsSymEntry(d.location_key()->value(), d.target_key()->value())));
}
