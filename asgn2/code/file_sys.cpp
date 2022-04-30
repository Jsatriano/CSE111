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
   // deletes tree
   return;
}

const string& inode_state::prompt() const { return prompt_; }

void inode_state::prompt (const string& new_prompt) {
   prompt_ = new_prompt;
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
   inode_ptr copy = this;
   base_file_ptr contents = copy->get_contents();
   if(auto d = dynamic_pointer_cast<directory>(contents); d) {
      cout << "is a directory" << endl;
      return false;
   }
   else {
      cout << "is a file" << endl;
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
   for (int i = 0; i < this->data.size(); i++) {
      size += this->data[i].length();
   }
   size += (this->data.size()-1); // accounts for spaces between words
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
}

size_t directory::size() const {
   size_t size {0};
   size = this->get_dirents().size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   if (this->is_type() == true) {
      this->get_dirents().erase(filename);
   }
   else if (this->is_type() == false) {
      if (this->get_dirents().size() <= 2) {
         inode_ptr extra = this->get_dirents().find("..");
         this->get_dirents().clear(); // ?
         extra->get_dirents().erase(filename);
      }
      else {
         cout << "remove: folder not empty" << endl;
      }
   }
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   nd = make_shared<inode> (file_type::DIRECTORY_TYPE);
   directory_entries& dirents = nd->get_dirents();
   dirents.insert (dirent_type (".", nd));
   dirents.insert (dirent_type ("..", this));
   directory_entries& Pdirents = this->get_dirents();
   Pdirents.insert (dirent_type (dirname, nd);
   return nd;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   nf = make_shared<inode> (file_type::PLAIN_TYPE);
   directory_entries& dirents = this->get_dirents();
   dirents.insert (dirent_type (filename, nf); // this is wrong
   return nf;
}

directory_entries& directory::get_dirents() {
   return dirents;
}

