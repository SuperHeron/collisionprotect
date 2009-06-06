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

#ifndef __OWNER_FINDER_HH__
#define __OWNER_FINDER_HH__

#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>

#include "CollisionProtect.hh"

class OwnerFinder
{
    public:
        OwnerFinder(std::string, std::tr1::shared_ptr<const paludis::PackageDepSpec> &, FilesByPackage*);
        void find(const paludis::FSEntry &);
        bool isFound() const;
        void visit(const paludis::ContentsFileEntry &);
        void visit(const paludis::ContentsDirEntry &);
        void visit(const paludis::ContentsOtherEntry &);
        void visit(const paludis::ContentsSymEntry &);
    private:
        std::string fileToFind;
        std::tr1::shared_ptr<const paludis::PackageDepSpec> depSpec;
        FilesByPackage* collisions;
        bool found;
};

#endif // __OWNER_FINDER_HH__
