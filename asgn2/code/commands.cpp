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
   //auto y = state.get_cwd()->get_dirents().find(words[1]);
   //if(y->second->is_type() == false) {
   //   throw command_error("cat: /: is a directory");
   //}
   else if(words.size() > 1) {
      inode_ptr T = state.get_cwd();
      inode_ptr S = state.get_cwd();
      wordvec name = split(words[1], "/");
      if(name.size() == 1) {
         auto x = T->get_dirents().find(name[0]);
         if(x == T->get_dirents().end()) {
            cout << "cat: path doesn't exist" << endl;
            return;
         }
         else {
            if(x->second->is_type() == false) {
               throw command_error("cat: cannot cat a directory");
            }
            else {
               cout << x->second->get_contents()->readfile() << endl;
               return;
            }
         }
      }
      else {
         for(unsigned int i = 0; i < name.size() - 1; i += 1) {
            auto x = T->get_dirents().find(name[i]);
            if(x == T->get_dirents().end()) {
               cout << "cat: path doesn't exist" << endl;
               return;
            }
            else {
               T = x->second;
            }
         }
         auto x = T->get_dirents().find(name[name.size() - 1]);
         if(x == T->get_dirents().end()) {
            cout << "cat: path doesn't exist" << endl;
            return;
         }
         else {
            if(x->second->is_type() == false) {
               throw command_error("cat: cannot cat a directory");
            }
            else {
               cout << x->second->get_contents()->readfile() << endl;
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
      inode_ptr T = state.get_cwd();
      inode_ptr S = state.get_cwd();
      wordvec name = split(words[1], "/");
      string new_path = state.get_path();
     
      for(unsigned int i = 0; i < name.size(); i += 1) {
         auto x = T->get_dirents().find(name[i]);
         if (x == T->get_dirents().end()) {
            cout << "cd: path doesn't exist" << endl;
            return;
         }
         else {
            T = x->second;
            if (T->is_type() == false) {
               if (x->first == "..") {
                  wordvec g = split(new_path, "/");
                  new_path.clear();
                  for (unsigned int u = 0; u < g.size() - 1; u++) {
                     new_path += ("/" + g[u]);
                  }
               }
               else if (new_path.size() == 1 and x->first != ".") {
                  new_path += name[i];
               }
               else if (new_path.size() != 1 and x->first != "."){
                  new_path += "/" + name[i];
               }
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
   if(words[1] == "0" or words.size() == 1) {
      exec::status(0);
   }
   else if(atoi(words[1].c_str())) {
      int x = atoi(words[1].c_str());
      exec::status(x);
   }
   else {
      exec::status(127);
   }

   throw ysh_exit();
}

// helper function for ls
//  - prints contents of the directory
void print_ls(inode_state& state, auto dir_path) {
   // if at root
   if(state.get_cwd() == state.get_root()) {
      cout << "/:" << endl;
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
      else if(item.second->is_type() == true) {
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
      cout << "in for loop" << endl;
      cout << "directories: " << directories << endl;
      if(words[1] == "/") {
         state.set_cwd(state.get_root());
         print_ls(state, words[1]);
         state.set_cwd(retn_dir);
         return;
      }
      for(auto dir_path: directories) {
         count += 1;
         cout << "hello" << endl;
         cout << dir_path << endl;
         for(auto item: state.get_cwd()->get_contents()->get_dirents()) {
            auto end = state.get_cwd()->get_contents()->get_dirents().rbegin();
            if(dir_path == "/") {
               cout << "if(dir_path == '/')" << endl;
               state.set_cwd(state.get_root());
               print_ls(state, dir_path);
               state.set_cwd(retn_dir);
               break;
            }
            else if(item.first == dir_path and item.second->is_type() == true) {
               cout << "poop" << endl;
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
            else if(item.first == dir_path) {
               if(count == end_path) {
                  state.set_cwd(item.second);
                  //fn_cd(state, directories);
                  cout << state.get_path() << endl;
                  print_ls(state, state.get_path());
                  state.set_cwd(retn_dir);
               }
               else {
                  state.set_cwd(item.second);
               }
               break;
            }
            else if(item.first == end->first) {
               cout << "myself" << endl;
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
   wordvec h;
   inode_ptr T = state.get_cwd();
   print_ls(state, state.get_path());
   if(state.get_cwd()->get_dirents().size() > 2) {
      for(auto i = state.get_cwd()->get_dirents().begin();
         i != state.get_cwd()->get_dirents().end(); i++) {

         if(i->first != "." and i->first != "..") {
            if(i->second->is_type() == false) {
               h.clear();
               h.push_back("lsr");
               h.push_back(i->first);
               fn_cd(state, h);
               lsr_work(state, h);
            }
         }
      }
   }
   if(state.get_cwd() != state.get_root()) {
      h.clear();
      h.push_back("lsr");
      h.push_back("..");
      fn_cd(state, h);
   }
   
}

void fn_lsr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr T = state.get_cwd();
   wordvec h;
   wordvec name;
   if(words.size() == 1) {
      h.clear();
      h.push_back("lsr");
      h.push_back(".");
      fn_cd(state, h);
      lsr_work(state, h);
      //print_ls(state, state.get_path());
      return;
   }
   if(state.get_cwd() != state.get_root()) {
      name = split(words[1], "/");
   }
   else {
      name.push_back(words[1]);
   }
   string file_name = name[name.size() - 1];

   if(name.size() <= 1) {
      if(T != state.get_root()) {
         auto y = T->get_dirents().find(file_name);
         if(y == T->get_dirents().end()) {
            cout << file_name << "doesn't exist" << endl;
         }
         else if(y->second->is_type() == true) {
            cout << "unable to call lsr on file" << endl;
         }
         else {
            h.clear();
            h.push_back("lsr");
            h.push_back(y->first);
            fn_cd(state, h);
         }
      }
      lsr_work(state, h);
   }
   else {
      unsigned int i = 0;
      for(i = 0; i < name.size() - 1; i += 1) {
         auto y = T->get_dirents().find(name[i]);
         if(y == T->get_dirents().end() or
            (i < name.size() - 1 and y->second->is_type() == true)) {
            
            cout << "path name doesn't exist" << endl;
            return;
         }
         else {
            h.clear();
            h.push_back("lsr");
            h.push_back(y->first);
            fn_cd(state, h);
         }
      }
      auto y = state.get_cwd()->get_dirents().find(name[i]);
      if(y == state.get_cwd()->get_dirents().end()) {
         cout << "path name doesn't exist" << endl;
         return;
      }
      else if(y->second->is_type() == true) {
         cout << "unable to call lsr on file" << endl;
      }
      else {
         h.clear();
         h.push_back("lsr");
         h.push_back(y->first);
         fn_cd(state, h);
      }
      lsr_work(state, h);
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

   string u = "";
   auto a = cwdir->get_dirents().find(".");
   auto b = cwdir->get_dirents().find("..");
   for(auto &z: b->second->get_dirents()) {
      if (z.second == a->second) {
         u += z.first;
      }
   }
   // starts at 2 because that is first word after pathname
   for(unsigned int i = 2; i != words.size(); i += 1) {
         temp.push_back(words[i]);
   }

   wordvec name = split(words[1], "/");
   inode_ptr T = state.get_cwd();
   
   string dir_name = "";
   dir_name += name[name.size()-1];
   if(name.size() > 1) {
      unsigned int i = 0;
      for (i = 0; i < name.size()-1; i++) {
         auto x = T->get_dirents().find(name[i]);
         if (x == T->get_dirents().end()) {
            cout << "mk: pathname doesn't exist" << endl;
            return;
         }
         T = x->second;
      }
      auto x = T->get_dirents().find(name[i]);
      if (x != T->get_dirents().end() or u == dir_name) {
         if (u == dir_name or x->second->is_type() == false) {
            cout << "mk: " << dir_name << " already exists" << endl;
            return;
         }
         else {
            x->second->get_contents()->writefile(temp);
         }
      }
      else {
         T->get_contents()->mkfile(name[name.size()-1]);
         x = T->get_dirents().find(name[i]);
         x->second->get_contents()->writefile(temp);
      }
   }
   else {
      auto x = T->get_dirents().find(dir_name);
      if (x != T->get_dirents().end() or u == dir_name) {
         if (u == dir_name or x->second->is_type() == false) {
            cout << "mk: " << dir_name << ": already exists as a directory" << endl;
            return;
         }
         else {
            x->second->get_contents()->writefile(temp);
         }
      }
      else {
         T->get_contents()->mkfile(dir_name);
         x = T->get_dirents().find(name[0]);
         x->second->get_contents()->writefile(temp);
      }
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

   string u = "";
   auto a = extra->get_dirents().find(".");
   auto b = extra->get_dirents().find("..");
   for(auto &z: b->second->get_dirents()) {
      if (z.second == a->second) {
         u += z.first;
      }
   }

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
         else {
            T = x->second;
         }
      }
      auto x = T->get_dirents().find(name[i]);
      if (x != T->get_dirents().end() or u == name[i]) {
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
      if (x != T->get_dirents().end() or u == dir_name) {
         // if name exists throws error
         cout << "mkdir: " << dir_name << ": already exists" << endl;
         return;
      }
      else {
         T->get_contents()->mkdir(dir_name);
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
      return;
   }
   string full_path = "";
   full_path += state.get_path();
   cout << full_path << endl;
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
         else {
            T = x->second;
         }
      }
      auto x = T->get_dirents().find(name[i]);
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

void rmr_work(inode_state& state, const wordvec& words) {
   wordvec h;
   inode_ptr T = state.get_cwd();
   if (state.get_cwd()->get_dirents().size() > 2) {
      for (auto i = state.get_cwd()->get_dirents().begin();
         i != state.get_cwd()->get_dirents().end(); i++) {
         cout << "file is " << i->first << endl;
         if (i->first != "." and i->first != "..") {
            if (i->second->is_type() == true) {
               // delete file
               cout << "delete file" << endl;
               state.get_cwd()->get_contents()->remove(i->first);
            }
            else if (i->second->is_type() == false) { // is a directory
               h.clear();
               h.push_back("rmr");
               h.push_back(i->first);
               fn_cd(state, h);
               rmr_work(state, h);
            }
         }
      }
   }
   if (state.get_cwd() != state.get_root()) {
      //auto x = state.get_cwd()->get_dirents().find("..");
      h.clear();
      h.push_back("rmr");
      h.push_back("..");
      fn_cd(state, h);
      for (auto i = state.get_cwd()->get_dirents().begin(); 
      i != state.get_cwd()->get_dirents().end(); i++) {
         if (i->second == T) {
            cout << "dir is " << i->first << endl;
         }
      }
      T->get_dirents().clear();
      cout << "delete dir" << endl;
      state.get_cwd()->get_dirents().erase(words[1]);
   }
}

void fn_rmr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr T = state.get_cwd();
   wordvec name = split(words[1], "/");
   string file_name = name[name.size()-1];
   wordvec h;
   if (words.size() == 1) {
      cout << "insuffient arguments" << endl;
      return;
   }
   if (name.size() <= 1) {
      auto y = T->get_dirents().find(file_name);
      if (y == T->get_dirents().end()) {
         cout << file_name << " doesn't exist" << endl;
      }
      else if (y->second->is_type() == true) {
         T->get_contents()->remove(y->first);
         return;
      }
      else {
         h.clear();
         h.push_back("rmr");
         h.push_back(y->first);
         fn_cd(state, h);
      }
      rmr_work(state, h);
   }
   else {
      unsigned int i = 0;
      for (i = 0; i < name.size()-1; i++) {
         cout << name[i] << endl;
         auto y = T->get_dirents().find(name[i]);
         if (y == T->get_dirents().end() or 
         (i < name.size()-1 and y->second->is_type() == true)) {
            cout << "path name doesnt exist" << endl;
            return;
         }
         else {
            h.clear();
            h.push_back("rmr");
            h.push_back(y->first);
            fn_cd(state, h);
         }
      }
      auto y = state.get_cwd()->get_dirents().find(name[i]);
      cout << name[i] << endl;
      if (y == state.get_cwd()->get_dirents().end()) {
         cout << "path name doesn't exist" << endl;
         return;
      }
      else if (y->second->is_type() == true) {
         state.get_cwd()->get_contents()->remove(y->first);
      }
      else {
         h.clear();
         h.push_back("rmr");
         h.push_back(y->first);
         fn_cd(state, h);
      }
      rmr_work(state, h);
   }
}

