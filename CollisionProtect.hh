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

#ifndef __COLLISION_PROTECT_HH__
#define __COLLISION_PROTECT_HH__

#include <map>
#include <vector>

#include <paludis/package_id.hh>

#include <paludis/util/fs_entry.hh>

/**
 * Typedefs
 */
//typedef std::multimap<paludis::FSEntry, std::tr1::shared_ptr<const paludis::PackageID> > FilesList;
typedef std::multimap<std::tr1::shared_ptr<paludis::ContentsEntry>, std::tr1::shared_ptr<const paludis::PackageID> > FilesList;
typedef std::map<paludis::FSEntry, bool> FSEntryList;
typedef std::map<std::tr1::shared_ptr<const paludis::PackageID>, std::vector<paludis::FSEntry> > FilesByPackage;
typedef std::map<paludis::FSEntry, std::tr1::shared_ptr<paludis::ContentsEntry> > ContentsList;

#endif // __COLLISION_PROTECT_HH__
