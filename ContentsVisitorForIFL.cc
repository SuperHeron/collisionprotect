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

#include "ContentsVisitorForIFL.hh"

ContentsVisitorForIFL::ContentsVisitorForIFL(std::string root, FilesList* ifl, const std::tr1::shared_ptr<const paludis::PackageID>& pkgID)
{
    this->root = root;
    this->ifl = ifl;
    this->pkgID = pkgID;
}

//void ContentsVisitorForIFL::visit(const paludis::ContentsDevEntry & d)
//{
//}

//void ContentsVisitorForIFL::visit(const paludis::ContentsMiscEntry & d)
//{
//}

void ContentsVisitorForIFL::visit(const paludis::ContentsFileEntry & d)
{
    this->ifl->insert(std::make_pair(new paludis::ContentsFileEntry(d.location_key()->value()), this->pkgID));
}

void ContentsVisitorForIFL::visit(const paludis::ContentsDirEntry & d)
{
}

//void ContentsVisitorForIFL::visit(const paludis::ContentsFifoEntry & d)
//{
//}

void ContentsVisitorForIFL::visit(const paludis::ContentsOtherEntry & d)
{
}

void ContentsVisitorForIFL::visit(const paludis::ContentsSymEntry & d)
{
    this->ifl->insert(std::make_pair(new paludis::ContentsSymEntry(d.location_key()->value(), d.target_key()->value()), this->pkgID));
}
