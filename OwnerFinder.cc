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

#include <iostream>
#include <paludis/util/stringify.hh>

#include "OwnerFinder.hh"

OwnerFinder::OwnerFinder(std::string fileToFind, const std::tr1::shared_ptr<const paludis::PackageID> & pkgID, FilesByPackage * collisions)
{
    this->fileToFind = fileToFind;
    this->pkgID = pkgID;
    this->collisions = collisions;
    this->found = false;
}

void OwnerFinder::find(const paludis::FSEntry & fsEntry)
{
	if(fileToFind == paludis::stringify(fsEntry))
	{
		this->found = true;
		FilesByPackage::iterator fbpIt = this->collisions->find(this->pkgID);
		if(fbpIt == this->collisions->end())
		{
			std::vector<paludis::FSEntry> vector;
			vector.push_back(fsEntry);
			this->collisions->insert(std::make_pair(this->pkgID, vector));
		}
		else
			fbpIt->second.push_back(fsEntry);
	}
}

bool OwnerFinder::isFound() const
{
	return found;
}

void OwnerFinder::visit(const paludis::ContentsFileEntry & e)
{
	paludis::FSEntry entry(e.location_key()->value());
	this->find(entry);
}

void OwnerFinder::visit(const paludis::ContentsDirEntry & e)
{
}

void OwnerFinder::visit(const paludis::ContentsOtherEntry & e)
{
}

void OwnerFinder::visit(const paludis::ContentsSymEntry & e)
{
	paludis::FSEntry entry(e.location_key()->value());
	this->find(entry);
}