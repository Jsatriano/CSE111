//*************************************************
// Justin Satriano    (jsatrian)
// Sriharsha Madala   (srmadala)
//*************************************************

// $Id: file_sys.cpp,v 1.13 2022-01-26 16:10:48-08 - - $

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

ostream& operator<< (ostream& out, file_type type) {
   switch (type) {
      case file_type::PLAIN_TYPE: out << "PLAIN_TYPE"; break;
      case file_type::DIRECTORY_TYPE: out << "DIRECTORY_TYPE"; break;
      default: assert (false);
   };
   return out;
}

inode_state::inode_state() {
   root = cwd = make_shared<inode> (file_type::DIRECTORY_TYPE);
   directory_entries& dirents = root->get_dirents();
   dirents.insert (dirent_type (".", root));
   dirents.insert (dirent_type ("..", root));
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
           << ", prompt = \"" << prompt() << "\""
           << ", file_type = " << root->contents->file_type());
}

inode_state::~inode_state() {
   inode_ptr T = root;
   inode_ptr U = root;
   inode_ptr V;
   if (root->get_dirents().size() > 2) {
      while ((U->get_dirents().size() > 2) or (U != root)) {
         for (auto i = U->get_dirents().begin();
         i != U->get_dirents().end(); i++) {
            if (i->first != "." and i->first != "..") {
               if (i->second->is_type() == false) {
                  U = i->second;
                  break;
               }
               else {
                  U->get_dirents().erase(i->first);
               }
            }
         }
         if ((U->get_dirents().size() == 2) and U != root) {
            V = U;
            auto x = U->get_dirents().find("..");
            U = x->second;
            V->get_dirents().clear();
            for (auto i = U->get_dirents().begin();
            i != U->get_dirents().end(); i++) {
               if (i->second == V) {
                  U->get_dirents().erase(i->first);
                  break;
               }
            }
         }
      }
   }
   root->get_dirents().clear();
   return;
}

const string& inode_state::prompt() const { return prompt_; }

void inode_state::prompt (const string& new_prompt) {
   prompt_ = new_prompt;
}

void inode_state::set_path (const string& new_path) {
   path = new_path;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
      default: assert (false);
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

size_t inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

directory_entries& inode::get_dirents() {
   return contents->get_dirents();
}

bool inode::is_type() const {
   //inode_ptr copy = this;
   base_file_ptr C = this->get_contents();
   if(auto d = dynamic_pointer_cast<directory>(C); d) {
      //cout << "is a directory" << endl;
      return false;
   }
   else {
      //cout << "is a file" << endl;
      return true;
   }
}



file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + file_type());
}

directory_entries& base_file::get_dirents() {
   throw file_error ("is a " + file_type());
}



size_t plain_file::size() const {
   size_t size {0};
   for (unsigned int i = 0; i != this->data.size(); i++) {
      size += this->data[i].length();
   }
   if (this->data.size() != 0) {
      size += (this->data.size()-1); // accounts for spaces between words
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data.clear();

   if(words.size() > 0) {
      for(unsigned int i = 0; i < words.size(); i += 1) {
         data.push_back(words[i]);
      }
   }
   if (words.size() == 0) {
      data.clear();
   }
}

size_t directory::size() const {
   size_t size {0};
   size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   auto x = this->get_dirents().find(filename);
   inode_ptr a = x->second;
   if (a->is_type() == true) {
      this->get_dirents().erase(filename);
   }
   else if (a->is_type() == false) {
      if (a->get_dirents().size() <= 2) {
         a->get_dirents().clear(); // ?
         this->get_dirents().erase(filename);
      }
      else {
         cout << "remove: folder not empty" << endl;
      }
   }
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr nd = make_shared<inode> (file_type::DIRECTORY_TYPE);
   auto x = this->get_dirents().find(".");
   inode_ptr T = x->second;
   directory_entries& nd_dirents = nd->get_dirents();
   nd_dirents.insert (dirent_type (".", nd));
   nd_dirents.insert (dirent_type ("..", T));
   directory_entries& Pdirents = this->get_dirents();
   Pdirents.insert (dirent_type (dirname, nd));
   return nd;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr nf = make_shared<inode> (file_type::PLAIN_TYPE);
   directory_entries& nf_dirents = this->get_dirents();
   nf_dirents.insert (dirent_type (filename, nf)); // this is wrong
   return nf;
}

directory_entries& directory::get_dirents() {
   return dirents;
}

