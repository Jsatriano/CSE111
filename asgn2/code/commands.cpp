// $Id: commands.cpp,v 1.27 2022-01-28 18:11:56-08 - - $

#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <string>

#include "commands.h"
#include "debug.h"

const command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result {cmd_hash.find (cmd)};
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such command");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status {exec::status()};
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}


void fn_cat (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // if a file is not given
   if(words.size() == 1) {
      throw command_error("cat: no file given");
   }
   // if word given is a directory
   else if(words[1] == "/") {
      throw command_error("cat: /: is a directory");
   }
   else if(words.size() > 1) {
      shared_ptr<directory> old_file = dynamic_pointer_cast<directory> (state.get_cwd()->get_contents());
      for(int i = 1; i < words.size(); i += 1) {
         inode_ptr new_file = old_file->get_file(words[i]);
         if(new_file == inode_ptr()) {
            throw command_error("cat: no such file exists");
         }
         else {
            shared_ptr<plain_file> file = dynamic_pointer_cast<plain_file> (file_content->get_contents());
            cout << file->readfile() << endl;
         }
      }
   }
}

void fn_cd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words[1] == "/" or words.size() == 1) {
      state.set_cwd(state.get_root());
      return;
   }
   else if(words.size() > 2) {
      throw command_error("cd: cd takes in only 1 argument");
   }
   else {
      bool is_dir = true;
      shared_ptr<directory> cwdir = dynamic_pointer_cast<directory> (state.get_cwd()->get_contents());
      inode_ptr next = dynamic_pointer_cast<inode> (cwdir);
      wordvec directories = split(words[1], "/");
      for(int i = 0; i < directories.size(); i += 1) {
         next = dynamic_pointer_cast<inode> (cwdir->get_file(directories[i]));
         if(next->is_type() or next == inode_ptr()) {
            if(next == inode_ptr()) {
               is_dir = false;
               break;
            }
            cwdir = dynamic_pointer_cast<directory> (cwdir->get_file(directories[i])->get_contents());
         }
         else {
            is_dir = false;
            break;
         }
      }
      if(is_dir = false) {
         throw command_error("cd: unable to reach given directory");
      }
      else {
         state.set_cwd(next);
      }
   }

}

void fn_echo (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}

void fn_exit (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   exit_status path_exit;
   if(words[1] == 0 or words.size() == 1) {
      path_exit.set(0);
   }
   else if(atoi(words[1].c_str())) {
      path_exit.set(atoi(words[1].c_str()));
   }
   else {
      path_exit.set(127);
   }

   throw ysh_exit();
}

// helper function for ls
//  - prints contents of the directory
void print_ls(inode_state& state, auto dir_path) {
   // if at root
   if(state.get_cwd() == state.get_root()) {
      cout << state.get_cwd()->get_path() + ":" << endl;
   }
   // if not at root
   else {
      cout << dir_path << ":" << endl;
   }
   // MIGHT NEED TO CHANGE HOW ITEMS PRINT OUT!!!!!!!
   for(auto item: state.get_cwd()->get_contents()->get_dirents()) {
      cout << setw(6) << item.second->get_inode_nr();
      cout << setw(6) << item.second->get_contents()->size();
      cout << "  ";
      //edge cases where "/" is not required
      if(item.first == "." or item.first == "..") {
         cout << item.first << endl;
      }
      else if(item.second->is_file) {
         cout << item.first << endl;
      }
      else {
         cout << item.first << "/" << endl;
      }
   }
}

void fn_ls (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   DEBUGS ('l', 
      const auto& dirents = state.get_root()->get_dirents();
      for (const auto& entry: dirents) {
         cerr << "\"" << entry.first << "\"->" << entry.second << endl;
      }
    );

   wordvec directories = split(words[1], "/");
   inode_ptr retn_dir;
   inode_ptr strt_dir;
   bool is_root = false;
   int count = 0;
   int end_path;

   retn_dir = state.get_cwd();
   if(words.size() == 1) {
      print_ls(state, state.get_cwd()->get_path());
      return;
   }

   for(int i = 1; i < words.size(); i += 1) {
      count = 0;
      end_path = directories.size();
      if(is_root == true) {
         state.set_cwd(state.get_root());
      }
      for(auto dir_path: directories) {
         count += 1;
         for(auto item: state.get_cwd()->get_contents()->getdirents()) {
            auto end = state.get_cwd()->get_contents()->get_dirents().rbegin();
            if(dir_path == "/") {
               state.set_cwd(state.get_root());
               print_ls(state, dir_path);
               state.set_cwd(retn_dir);
               break;
            }
            else if(item.first == sub_path and item.second->is_file) {
               if(count != end_path) {
                  cout << words[0] << " is an invalid name" << endl;
                  state.set_cwd(retn_dir);
               }
               else {
                  cout << dir_path << endl;
                  state.set_cwd(retn_dir);
               }
               break;
            }
            else if(item.first == dir_path) { // we not at the end
               state.set_cwd(item.second);
               break;
            }
            else if(item.first == dir_path and count == end_path) { // we at the end
               state.set_cwd(item.second);
               print_ls(state, state.get_cwd()->get_path());
               state.set_cwd(retn_dir);
               break;
            }
            else if(item.first == end->first) {
               cout << words[0] << "cannot access" << dir_path << ": No such file or directory" << endl;
               state.set_cwd(retn_dir);
               break;
            }
         }
      }
      
   }
}

void fn_lsr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}


void fn_make (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_prompt (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

