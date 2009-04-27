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

#include <paludis/paludis.hh>
#include <tr1/memory>
#include <typeinfo>

#include "pstream.h"

//#include "ContentsVisitorForIFL.hh"
#include "ContentsVisitorForIPFL.hh"
#include "CollisionProtect.hh"
#include "OwnerFinder.hh"

//const std::tr1::shared_ptr<const paludis::Sequence<std::string> > paludis_hook_auto_phases(const paludis::Environment *env)
//{
//	std::tr1::shared_ptr<paludis::Sequence<std::string> > phases(new paludis::Sequence<std::string>());
//	phases->push_back("merger_check_post");
//	return phases;
//}

std::string canonicalize_path(std::string path)
{
	size_t pos = path.find("/../");
	if(pos >= 3)
	{
		while(pos != std::string::npos)
		{
			size_t pos2 = path.rfind("/", pos - 1);
			path.erase(pos2, pos - pos2 + 3);
			pos = path.find("/../");
		}
	}
	pos = path.find("//");
	while(pos != std::string::npos)
	{
		path.erase(pos, 1);
		pos = path.find("//");
	}
	return path;
}

/**
 * Get an environment variable defined in bashrc_files() like /etc/paludis/bashrc
 * @param hook Current hook
 * @param key Name of environment variable
 * @return Value of environment variable
 */
std::string get_envvar_from_bashrc(const paludis::Hook& hook, const std::string& key)
{
	std::istringstream bashrc_ss(hook.get("PALUDIS_BASHRC_FILES"));
	std::ostringstream command;
	std::string buffer;
	while(bashrc_ss >> buffer)
		command << "source " << buffer << " && ";
	command << "echo $" << key;
	redi::ipstream cmd_ss(command.str());
	getline(cmd_ss, buffer);
	return buffer;
}

/**
 * Check whether a file reside in a directory of COLLISION_IGNORE
 * @param file File to check
 * @param vector COLLISION_IGNORE directories
 * @return whether file is in a COLLISION_IGNORE directory or not
 */
bool is_in_collision_ignore(const paludis::FSEntry& file, std::vector<std::string>& vector)
{
	bool returnBool = false;
	for(std::vector<std::string>::const_iterator path(vector.begin()), path_end(vector.end()); path != path_end; ++path)
	{
		if(paludis::stringify(file.dirname()).find(*path, 0) == 0 || paludis::stringify(file).find(*path, 0) == 0)
		{
			returnBool = true;
			break;
		}
	}
	return returnBool;
}

/**
 * Collect all files recursively in a directory
 * @param directory Directory to start collecting from
 * @param path_to_strip Leading path to remove
 * @param list List of files
 * @param collIgnore ${COLLISION_IGNORE} and friends directories
 */
void iterate_over_directory(const paludis::FSEntry& directory, const paludis::FSEntry& path_to_strip, FSEntryList* list, std::vector<std::string>& collIgnore, std::string root)
{
	for(paludis::DirIterator di(directory), di_end; di != di_end; ++di)
	{
		if(di->is_directory())
			iterate_over_directory(*di, path_to_strip, list, collIgnore, root);
		else
		{
			paludis::FSEntry entry(paludis::FSEntry(root)/di->strip_leading(path_to_strip));
			if(is_in_collision_ignore(entry, collIgnore))
				list->insert(std::make_pair(entry, false));
			else
				list->insert(std::make_pair(entry, entry.exists()));
		}
	}
}

/**
 * Fill the COLLISION_IGNORE vector with a variable containing directories to discard
 * @param vector The actual COLLISION_IGNORE vector
 * @param variable The variable containing directories to discard
 */
void fill_collision_ignore_with_variable(std::vector<std::string> *vector, std::string variable)
{
	std::istringstream iss(variable);
	std::string path;
	while(iss >> path)
	{
		if(std::find(vector->begin(), vector->end(), path) == vector->end())
			vector->push_back(path);
	}
}

/**
 * Compare ${IMAGE} files list with installed package files list
 * @param imageList ${IMAGE} files list
 * @param pkgList installed package files list
 * @return true if empty or no colliding files left
 */
bool compareFilesList(FSEntryList& imageList, ContentsList& pkgList)
{
//	std::cout << "Comparison..." << std::endl;
    bool returnBool = true;
    int count = 0;
    if(!imageList.empty())
    {
    	std::cout << imageList.size() << " files to check" << std::endl;
        for(FSEntryList::iterator imgFS(imageList.begin()), imgFS_end(imageList.end()); imgFS != imgFS_end; ++imgFS)
        {
			count++;
			if(count > 0 && count % 500 == 0)
				std::cout << "...on " << count << "th target..." << std::endl;
        // For all files that exists
            if(imgFS->second)
            {
                bool isInPkg = false;
//                std::cout << imgFS->first;
//                if(imgFS->first.is_symbolic_link())
//					std::cout << " -> " << imgFS->first.readlink();
//				std::cout << std::endl;
                for(ContentsList::const_iterator pkgFS(pkgList.begin()), pkgFS_end(pkgList.end()); pkgFS != pkgFS_end; ++pkgFS)
                {
                    if(imgFS->first.realpath_if_exists() == (*pkgFS)->location_key()->value().realpath_if_exists())
                    {
//						std::cout << "Found file : " << (*pkgFS)->location_key()->value();
                        std::tr1::shared_ptr<paludis::ContentsSymEntry> cse(std::tr1::dynamic_pointer_cast<paludis::ContentsSymEntry>(*pkgFS));
                        if(cse && imgFS->first.is_symbolic_link())
                        {
//							std::cout << " -> " << cse->target_key()->value();
                            if(canonicalize_path(imgFS->first.readlink()) == canonicalize_path(cse->target_key()->value()))
                            {
                                imgFS->second = false;
                                isInPkg = true;
                            }
                        }
                        else if(!cse && !imgFS->first.is_symbolic_link())
                        {
                            imgFS->second = false;
                            isInPkg = true;
						}
//						std::cout << std::endl;
                    }
                    if(isInPkg)
                        break;
                }
                if(!isInPkg)
                    returnBool = false;
            }
        }
    }
	return returnBool;
}

/**
 * Find PackageID owner of a file
 * @param env Environment
 * @param fileName fileName to check
 * @param collisions Collisions map
 * @return whether file has found its owner or not
 */
bool find_owner(const paludis::Environment* env, std::string fileName, FilesByPackage * collisions)
{
	bool found_owner = false;
	for(paludis::PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()); r != r_end; ++r)
	{
		paludis::SupportsActionTest<paludis::InstalledAction> action_test;
		if((*r)->some_ids_might_support_action(action_test))
		{
			std::tr1::shared_ptr<const paludis::CategoryNamePartSet> cats((*r)->category_names());
			for(paludis::CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()); c != c_end; ++c)
			{
				std::tr1::shared_ptr<const paludis::QualifiedPackageNameSet> pkgs((*r)->package_names(*c));
				for(paludis::QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()); p != p_end; ++p)
				{
					std::tr1::shared_ptr<const paludis::PackageIDSequence> ids((*r)->package_ids(*p));
					for(paludis::PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()); v != v_end; ++v)
					{
						if ((*v)->contents_key())
						{
							std::tr1::shared_ptr<const paludis::Contents> contents((*v)->contents_key()->value());
							OwnerFinder finder(fileName, *v, collisions);
							std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), paludis::accept_visitor(finder));
							found_owner = finder.isFound();
							if(found_owner)
//							{
//								std::cout << "File found : " << fileName << std::endl;
								return found_owner;
//							}
						}
					}
				}
			}
		}
	}
	return found_owner;
}

/**
 * Function to run the current hook (declared in Paludis API)
 */
paludis::HookResult paludis_hook_run(const paludis::Environment* env, const paludis::Hook& hook)
{
    paludis::HookResult result = { paludis::value_for<paludis::n::max_exit_status>(0), paludis::value_for<paludis::n::output>("") };
/*
 * Showing all variables in hook
 * For debugging only
 */
//	for(paludis::Hook::ConstIterator h(hook.begin()), h_end(hook.end()); h != h_end; ++h)
//		std::cout << h->first << " : " << h->second << std::endl;
/*
 * Checking for variable $COLLISION_IGNORE
 * This allows to skip the check on several directories
 * Checks everything if not set
 * Checks nothing if set to or contains $ROOT
 */
	std::cout << std::endl;
//	std::cout << "Checking contents of ${COLLISION_IGNORE}..." << std::endl;
	std::string collisionIgnore = get_envvar_from_bashrc(hook, "COLLISION_IGNORE");
	std::istringstream collIgnore_iss(collisionIgnore);
	std::string root = hook.get("ROOT");
	bool canIgnore = false;
	std::string path;
	while(collIgnore_iss >> path)
	{
		if(path == root)
		{
			canIgnore = true;
			break;
		}
	}
	if(canIgnore)
	{
		std::ostringstream message;
		message << "${COLLISION_IGNORE} contains \"" << root << "\", skipping collision check";
		std::cout << message.str() << std::endl;
		result.output = paludis::value_for<paludis::n::output>(message.str());
		return result;
	}
	else
	{
//		std::cout << "Gathering directories in ${COLLISION_IGNORE}, ${CONFIG_PROTECT_MASK}, ${CONFIG_PROTECT} and info dirs..." << std::endl;
		std::vector<std::string> collIgnoreVector;
		fill_collision_ignore_with_variable(&collIgnoreVector, collisionIgnore);
		fill_collision_ignore_with_variable(&collIgnoreVector, hook.get("CONFIG_PROTECT_MASK"));
		fill_collision_ignore_with_variable(&collIgnoreVector, hook.get("CONFIG_PROTECT"));
		fill_collision_ignore_with_variable(&collIgnoreVector, "/usr/share/info/dir");
		std::string cmd("echo `find /usr/share/gcc-data/ -iname info -type d`/dir");
		std::string gccDataInfoDir;
		redi::ipstream cmd_ss(cmd);
		getline(cmd_ss, gccDataInfoDir);
		fill_collision_ignore_with_variable(&collIgnoreVector, gccDataInfoDir);
//		for(std::vector<std::string>::const_iterator cIVit(collIgnoreVector.begin()), cIVit_end(collIgnoreVector.end()); cIVit != cIVit_end; ++cIVit)
//            std::cout << *cIVit << std::endl;
        FilesList installedFilesList;
        ContentsList installedPkgFilesList;
 		FSEntryList imageFileList;
		FilesByPackage collisions;
		paludis::QualifiedPackageName packageName(paludis::CategoryNamePart(hook.get("CATEGORY")), paludis::PackageNamePart(hook.get("PN")));
		paludis::VersionSpecOptions vso;
		paludis::VersionSpec version(hook.get("PVR"), vso);
		paludis::SlotName slot(hook.get("SLOT"));
		std::tr1::shared_ptr<const paludis::PackageID> packageID;
		std::cout << "Checking for collisions..." << std::endl;
/*
 * Getting files from currently installing package
 */
//		std::cout << "Iterating over ${IMAGE} directory..." << std::endl;
		iterate_over_directory(hook.get("IMAGE"), hook.get("IMAGE"), &imageFileList, collIgnoreVector, root);
//		for(FSEntryList::const_iterator fs(imageFileList.begin()), fs_end(imageFileList.end()); fs != fs_end; ++fs)
//			std::cout << fs->first << std::endl;
/*
 * Find installed package being replaced
 */
//		std::cout << "Getting list of files of possibly old package version..." << std::endl;
//		std::tr1::shared_ptr<const paludis::PackageIDSequence> oldpkg((*env)[paludis::selection::BestVersionInEachSlot(paludis::generator::Package(packageName) | paludis::filter::SupportsAction<paludis::InstalledAction>())]);
		std::tr1::shared_ptr<const paludis::PackageIDSequence> oldpkg((*env)[paludis::selection::BestVersionInEachSlot(paludis::generator::Intersection(
																													   paludis::generator::Package(packageName),
																													   paludis::generator::InRepository(paludis::RepositoryName("installed"))
																													   ))]);
		for(paludis::PackageIDSequence::ConstIterator p(oldpkg->begin()), p_end(oldpkg->end()); p != p_end; ++p)
		{
			if((*p)->slot_key()->value() == slot)
			{
                ContentsVisitorForIPFL visitor(hook.get("ROOT"), &installedPkgFilesList);
                std::for_each(
                    paludis::indirect_iterator((*p)->contents_key()->value()->begin()),
                    paludis::indirect_iterator((*p)->contents_key()->value()->end()),
                    paludis::accept_visitor(visitor)
                );
			}
		}
//		std::cout << "List of files already installed by other version of package..." << std::endl;
//		for(ContentsList::const_iterator file(installedPkgFilesList.begin()), file_end(installedPkgFilesList.end()); file != file_end; file++)
//		{
//		    std::cout << (*file)->location_key()->value();
//            std::tr1::shared_ptr<paludis::ContentsSymEntry> cse(std::tr1::dynamic_pointer_cast<paludis::ContentsSymEntry>(*file));
//            if(cse)
//                std::cout << " -> " << cse->target_key()->value();
//            std::cout << " (" << typeid(*file).name() << ")" << std::endl;
//		}
/*
 * If there are no files involved in collision in IMAGE, tell the user that everything is OK
 * Otherwise, find out packages containing files involved in collision
 */
		if(compareFilesList(imageFileList, installedPkgFilesList))
		{
			std::string message("No collision detected, continuing");
			std::cout << message << std::endl;
			result.output = paludis::value_for<paludis::n::output>(message);
            return result;
		}
		else
		{
/*
 * Make packageID from CATEGORY, PN, PVR and SLOT and look for original repository
 */
			std::cout << "Collisions detected, please wait..." << std::endl;
//			std::cout << "Creating PackageID..." << std::endl;
			for(paludis::PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()); r != r_end; ++r)
			{
				std::tr1::shared_ptr<const paludis::PackageIDSequence> pkgs_from_repo((*r)->package_ids(packageName));
				for(paludis::PackageIDSequence::ConstIterator id(pkgs_from_repo->begin()), id_end(pkgs_from_repo->end()); id != id_end; ++id)
				{
					if((*id)->version() == version && (*id)->slot_key()->value() == slot)
						packageID = *id;
				}
			}
//			std::cout << "PkgID : " << packageID->canonical_form(paludis::idcf_full) << std::endl;
/*
 * Find owners of existing files (this can take a while)
 */
			for(FSEntryList::iterator file(imageFileList.begin()), file_end(imageFileList.end()); file != file_end; ++file)
			{
				if(file->second)
				{
					if(!find_owner(env, paludis::stringify(file->first), &collisions))
					{
//						std::cout << "File not found : " << file->first << std::endl;
/*
 * Add file to installing PackageID if orphaned
 */
						if(collisions.find(packageID) == collisions.end())
						{
							std::vector<paludis::FSEntry> vector;
							vector.push_back(file->first);
							collisions.insert(std::make_pair(packageID, vector));
						}
						else
							collisions[packageID].push_back(file->first);
					}
				}
			}
/*
 * Show each package and files involved in collision and abort installation
 */
//			for(FilesByPackage::iterator file(collisions.begin()), file_end(collisions.end()); file != file_end; ++file)
//			{
//				for(std::vector<paludis::FSEntry>::iterator fs(file->second.begin()), fs_end(file->second.end()); fs != fs_end; ++fs)
//				{
//					if(fs->getFileName() == root)
//						file->second.erase(fs);
//				}
//				if(file->second.empty())
//					collisions.erase(file);
			std::cout << "Detected collisions :" << std::endl;
			for(FilesByPackage::const_iterator file(collisions.begin()), file_end(collisions.end()); file != file_end; ++file)
			{
				if(file->first == NULL)
					std::cout << "	Other version of " << packageID->name() << " :" << std::endl;
				else if(file->first == packageID)
					std::cout << "	Orphaned files :" << std::endl;
				else
					std::cout << "	" << file->first->canonical_form(paludis::idcf_full) << " :" << std::endl;
				for(std::vector<paludis::FSEntry>::const_iterator fs(file->second.begin()), fs_end(file->second.end()); fs != fs_end; ++fs)
				{
					std::cout << "		" << *fs;
					if(fs->is_symbolic_link())
						std::cout << " -> " << fs->readlink();
					std::cout << std::endl;
				}
			}
//			std::cout << "Collisions with installing files :" << std::endl;
//			for(FSEntryList::const_iterator entry(imageFileList.begin()), entry_end(imageFileList.end()); entry != entry_end; ++entry)
//			{
//				if(entry->second)
//				{
//					std::cout << "  " << entry->first;
//					if(entry->first.is_symbolic_link())
//						std::cout << " -> " << entry->first.readlink();
//					std::cout << std::endl;
//				}
//			}
			std::string message("Collisions detected, aborting");
			std::cout << message << std::endl;
			result.max_exit_status = paludis::value_for<paludis::n::max_exit_status>(1);
			result.output = paludis::value_for<paludis::n::output>(message);
			return result;
		}
	}
}
