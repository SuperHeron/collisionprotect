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

OwnerFinder::OwnerFinder(std::string fileToFind, std::shared_ptr<const paludis::PackageDepSpec> & depSpec, FilesByPackage * collisions)
{
    this->fileToFind = fileToFind;
    this->depSpec = depSpec;
    this->collisions = collisions;
    this->found = false;
}

void OwnerFinder::find(const paludis::FSPath & fsPath)
{
	if(this->fileToFind == paludis::stringify(fsPath))
	{
		bool pkgFound = false;
		this->found = true;
		for(FilesByPackage::iterator fbpIt(this->collisions->begin()), fbpIt_end(this->collisions->end()); fbpIt != fbpIt_end && !pkgFound; ++fbpIt)
		{
			if(paludis::stringify(*(fbpIt->first)) == paludis::stringify(*(this->depSpec)))
			{
				pkgFound = true;
				fbpIt->second.push_back(fsPath);
			}
		}
		if(!pkgFound)
		{
			std::vector<paludis::FSPath> fsPathVector;
			fsPathVector.push_back(fsPath);
			this->collisions->insert(std::make_pair(this->depSpec, fsPathVector));
		}
	}
}

bool OwnerFinder::isFound() const
{
	return found;
}

void OwnerFinder::visit(const paludis::ContentsFileEntry & e)
{
	paludis::FSPath fsPath(e.location_key()->parse_value());
	this->find(fsPath);
}

void OwnerFinder::visit(const paludis::ContentsDirEntry & e)
{
}

void OwnerFinder::visit(const paludis::ContentsOtherEntry & e)
{
}

void OwnerFinder::visit(const paludis::ContentsSymEntry & e)
{
	paludis::FSPath fsPath(e.location_key()->parse_value());
	this->find(fsPath);
}
