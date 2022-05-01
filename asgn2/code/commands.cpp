// $Id: commands.cpp,v 1.27 2022-01-28 18:11:56-08 - - $

#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <string>
#include <iomanip>

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
      inode_ptr T = state.get_cwd();
      inode ptr S = state.get_cwd();
      wordvec name = split(words[1], "/");
      if(name.size() == 1) {
         auto x = T->get_dirents().find(name[0]);
         if(x == T->get_dirents().end()) {
            cout << "cat: path doesn't exist" << endl;
            return;
         }
         else {
            if(S->is_type == false) {
               throw command_error("cat: cannot cat a directory");
            }
            else {
               cout << S.get_contents()->readfile() << endl;
               return;
            }
         }
      }
      else {
         for(auto i = 0; i < name.size() - 1; i += 1) {
            auto x = T->get_dirents().find(name[i]);
            if(x == T->get_dirents().end()) {
               cout << "cat: path doesn't exist" << endl;
               return;
            }
            else {
               T = S;
            }
         }
         auto x = T->get_dirents().find(name[name.size() - 1]);
         if(x == T->get_dirents().end()) {
            cout << "cat: path doesn't exist" << endl;
            return;
         }
         else {
            if(S->is_type == false) {
               throw command_error("cat: cannot cat a directory");
            }
            else {
               cout << S.get_contents()->readfile() << endl;
               return;
            }
         } 
      }
   }
}

void fn_cd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words[1] == "/" or words.size() == 1) {
      state.set_cwd(state.get_root());
      state.set_path("/");
      return;
   }
   else if(words.size() > 2) {
      throw command_error("cd: cd takes in only 1 argument");
   }
   else {
      bool is_dir = false;
      inode_ptr T = state.get_cwd();
      indoe_ptr S = state.get_cwd();
      wordvec name = split(words[1], "/");
      string new_path = T->get_path();
     
      for(unsigned int i = 0; i < name.size(); i += 1) {
         auto x = T->get_dirents().find(name[i]);
         if (x == T->get_dirents().end()) {
            cout << "cd: path doesn't exist" << endl;
            return;
         }
         else {
            T = T->get_dirents().find(name[i]);
            if (T->is_type() == false) {
               new_path += "/" + name[i];
            }
            else {
               cout << "cd: cannot cd into file" << endl;
               return;
            }
         }
      }
      state.set_cwd(T);
      state.set_path(new_path);
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
   //status path_exit;
   if(words[1] == "0" or words.size() == 1) {
      //path_exit.set(0);
      throw ysh_exit(0);
   }
   else if(atoi(words[1].c_str())) {
      //path_exit.set(atoi(words[1].c_str()));
      throw ysh_exit(atoi(words[1].c_str()))
   }
   else {
      //path_exit.set(127);
      throw ysh_exit(127);
   }

   throw ysh_exit();
}

// helper function for ls
//  - prints contents of the directory
void print_ls(inode_state& state, auto dir_path) {
   // if at root
   if(state.get_cwd() == state.get_root()) {
      cout << state.get_path() << ":" << endl;
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
      print_ls(state, state.get_path());
      return;
   }

   for(unsigned int i = 1; i < words.size(); i += 1) {
      count = 0;
      end_path = directories.size();
      if(is_root == true) {
         state.set_cwd(state.get_root());
      }
      for(auto dir_path: directories) {
         count += 1;
         for(auto item: state.get_cwd()->get_contents()->get_dirents()) {
            auto end = state.get_cwd()->get_contents()->get_dirents().rbegin();
            if(dir_path == "/") {
               state.set_cwd(state.get_root());
               print_ls(state, dir_path);
               state.set_cwd(retn_dir);
               break;
            }
            else if(item->first == end_path and item.second->is_file) {
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
            else if(item->first == dir_path) { // we not at the end
               state.set_cwd(item.second);
               break;
            }
            else if(item->first == dir_path and count == end_path) { // we at the end
               state.set_cwd(item.second);
               print_ls(state, state.get_path());
               state.set_cwd(retn_dir);
               break;
            }
            else if(item->first == end->first) {
               cout << words[0] << "cannot access" << dir_path << ": No such file or directory" << endl;
               state.set_cwd(retn_dir);
               break;
            }
         }
      }     
   }
}

// helper function for fn_lsr
// recursive depth-first preorder traversal
void lsr_work(inode_state& state, const wordvec& words) {
   inode_ptr cwdir = state.get_cwd();
   wordvec vec;
   string str;

   if(words.size() <= 1) {
      str = words[0];
   }
   else {
      for(unsigned int i = 1; i < words.size(); i += 1) {
         if(i == words.size() - 1) {
            str += words[i];
         }
         else {
            str += words[i];
            str +=  "/";
         }
      }
   }
   if(vec[0] != "/") {
      vec.insert(0, "/");
   }

   bool is_dir = false;
   wordvec lsr_vec;
   wordvec cd_root;
   wordvec cd_parent;

   lsr_vec.clear();
   cd_root.push_back("..");
   cd_parent.push_back("/");
   // if stuff is already in str
   if(str != "/") {
      lsr_vec.push_back(str);
      fn_cd(state, lsr_vec);
      fn_ls(state, lsr_vec);
   }
   // else its empty
   else {
      lsr_vec.push_back("/");
      fn_cd(state, lsr_vec);
      fn_ls(state, lsr_vec);
   }

   for(auto item = state.get_cwd()->get_contents()->get_dirents().begin();
      item != state.get_cwd()->get_contents()->get_dirents().end(); item++) {
      
      if(item->second->is_type() == false) {
         if(item->first != "." and item->first != "..") {
            lsr_vec.clear();
            is_dir = true;
            lsr_vec.push_back(item->first);
            lsr_work(state, lsr_vec); //recursive call
            fn_cd(state, cd_parent);
         }
      }  
   }
   if(is_dir == false) {
      return;
   }
}

void fn_lsr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr cwdir = state.get_cwd();
   wordvec lsr_words;
   for(unsigned int i = 1; i < words.size(); i += 1) {
      lsr_words.push_back(words[i]);
      lsr_work(state, lsr_words);
      state.get_cwd() = cwdir;
   }
}


void fn_make (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1) {
      throw command_error("make: missing argument");
   }
   
   string str = "";
   wordvec temp;
   string file_name = words[1];
   inode_ptr cwdir = state.get_cwd();
   // starts at 2 because that is first word after pathname
   for(unsigned int i = 2; i < words.size(); i += 1) {
         temp.push_back(words[i]);
   }

   wordvec go_to;
   wordvec name = split(words[1], "/");

   go_to.push_back(file_name);
   if(name.size() > 1) {
      fn_cd(state, go_to);
      // if directory already exists, throw error
      if(state.get_cwd()->get_path() == "/" + file_name) {
         state.get_cwd() = cwdir;
         throw command_error ("make: directory exists: cannot create file");
      }
      state.get_cwd()->get_contents()->mkfile(name[name.size() - 1]);
      state.get_cwd()->get_contents()->writefile(temp);

      state.get_cwd() = cwdir;
   }
   else {
      for(auto item = cwdir->get_contents()->get_dirents().begin(); 
         item != cwdir->get_contents()->get_dirents().end(); item++) {
         
         if(item->first == file_name
            and item->second->is_type() == false) {
            
            state.get_cwd() = cwdir;
            throw command_error ("make:" + file_name + ": already exists");
         }
         else if(item->first == file_name) {
            item->second->get_contents()->writefile(temp);
            return;
         }
      }
      cwdir->get_contents()->mkfile(file_name);
      cwdir->get_contents()->writefile(temp);
      state.get_cwd() = cwdir;
   }
}

void fn_mkdir (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() == 1) {
      throw command_error("mkdir: missing argument");
   }
   else if (words[1] == "/") {
      throw command_error("mkdir: directory alrady exists"); // change
   }
   else if (words.size() > 2) {
      throw command_error("mkdir: more than one operand specified");
   }
   inode_ptr extra = state.get_cwd();

   string dir_name = "";
   inode_ptr T = state.get_cwd();
   inode_ptr S = state.get_cwd();

   wordvec name = split(words[1], "/");
   dir_name += name[name.size()-1];

   if (name.size() > 1) {
      unsigned int i = 0;
      for (i = 0; i < name.size()-1; i++) {
         auto x = T->get_dirents().find(name[i]);
         if (x == T->get_dirents().end()) {
            cout << "mkdir: pathname doesn't exist" << endl;
            return;
         }
      }
      auto x = T->get_dirents().find(name[i+1]);
      if (x != T->get_dirents().end()) {
         cout << "mkdir: " << dir_name << " already exists" << endl;
         return;
      }
      else {
         T->get_contents()->mkdir(name[name.size()-1]);
      }   
   }
   else {
         // checks if name already exists
         auto x = T->get_dirents().find(dir_name);
         if (x != T->get_dirents().end()) {
            // if name exists throws error
            cout << "mkdir: " << dir_name << ": already exists" << endl;
            return;
         }
         else {
            T->get_contents()->mkdir(dir_name);
         }
      }
   }
}

void fn_prompt (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() == 1) {
      throw command_error("prompt: missing argument");
   }
   string new_prompt = "";
   for (size_t i = 1; i < words.size(); i++) {
      new_prompt += words[i] + " ";
   }
   state.prompt(new_prompt);
}

void fn_pwd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() > 1) {
      throw command_error("pwd: too many arguments");
   }
   inode_ptr cwdir = state.get_cwd();
   if (state.get_cwd() == state.get_root()) {
      cout << "/" << endl;
   }
   string full_path = "";
   full_path += state.get_path();
   wordvec field = split(full_path, "/");
   string curr_path = "/" + field[field.size()-1];
   cout << curr_path << endl;
}

void fn_rm (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() == 1) {
      throw command_error("rm: missing argument");
   }
   else if (words[1] == "/") {
      throw command_error("rm: cannot remove root directory");
   }
   else if (words.size() > 2) {
      throw command_error("rm: too many arguments");
   }

   string file_name = words[1];
   inode_ptr extra = state.get_cwd();
   inode_ptr T = state.get_cwd();
   inode_ptr S = state.get_cwd();

   // used to check if the argument is a path and not just a filename
   wordvec name = split(words[1], "/");

   // if an entire path is specified instead of just one name
   if (name.size() > 1) {
      unsigned int i = 0;
      for (i = 0; i < name.size()-1; i++) {
         auto x = T->get_dirents().find(name[i]);
         if (x == T->get_dirents().end()) {
            cout << "rm: pathname doesn't exist" << endl;
            return;
         }
      }
      auto x = T->get_dirents().find(name[i+1]);
      if (x == T->get_dirents().end()) {
         cout << "rm: " << name[i+1] << " doesn't exists" << endl;
         return;
      }
      else {
         T->get_contents()->remove(name[name.size()-1]);
      }
   }
   else { // if pathname is only 1 word
      auto x = T->get_dirents().find(name[0]);
      if (x == T->get_dirents().end()) {
         cout << "rm: " << name[0] << " doesn't exists" << endl;
         return;
      }
      else {
         T->get_contents()->remove(name[0]);
      }
   }
}

void fn_rmr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr T = state.get_cwd();
   inode_ptr extra = T;
   wordvec name = split(words[1], "/");
   string file_name = name[name.size()-1];

   if (T->get_dirents().size() > 2) {
      for (auto i = state.get_cwd()->get_dirents().begin(); 
         i != state.get_cwd()->get_dirents().end(); i++) {
         if (i->first != "." or i->first != "..") {
            if (i->second->is_type() == true) {
               // delete file
               T->get_contents()->remove(i->first);
               T->get_dirents().erase(i->first);
            }
            else if (i->second->is_type() == false) { // is a directory
               wordvec x;
               x.push_back(i->first);
               fn_cd(state, x);
               fn_rmr(state, x);
            }
         }
      }
   }
   if (T != state.get_root()) {
      state.set_cwd(T->get_dirents().find(".."));
      T->get_dirents().clear();
      state.get_cwd()->get_dirents().erase(file_name);
   }

   // delete director because it is empty
   // set cwd back to parent
   // erase .. from directory map before deleting current directory
}

