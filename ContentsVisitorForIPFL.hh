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

#ifndef __CONTENTS_VISITOR_FOR_IPFL_HH__
#define __CONTENTS_VISITOR_FOR_IPFL_HH__

#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>

#include "CollisionProtect.hh"

class ContentsVisitorForIPFL
{
    public:
//        ContentsVisitorForIPFL(std::string, std::vector<FSDescriptor>*);
        ContentsVisitorForIPFL(std::string, ContentsList*);
//        void visit(const paludis::ContentsDevEntry & d);
//        void visit(const paludis::ContentsMiscEntry & d);
        void visit(const paludis::ContentsFileEntry & d);
        void visit(const paludis::ContentsDirEntry & d);
//        void visit(const paludis::ContentsFifoEntry & d);
        void visit(const paludis::ContentsOtherEntry & d);
        void visit(const paludis::ContentsSymEntry & d);
    private:
        std::string root;
//        std::vector<FSDescriptor>* ipfl;
        ContentsList* ipfl;
};

#endif // __CONTENTS_VISITOR_FOR_IPFL_HH__
