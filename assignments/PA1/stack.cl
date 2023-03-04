(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

class List {
   isNil() : Bool { true };

   head()  : String { { abort(); ""; } };

   tail()  : List { { abort(); self; } };

   newNode(i : String) : List {
      (new Node).init(i, self)
   };
   
};

class Node inherits List {

   value : String;	-- The element in this list cell

   next : List;	-- The rest of the list

   isNil() : Bool { false };

   head()  : String { value };

   tail()  : List { next };

   init(i : String, rest : List) : List {
      {
         value <- i;
         next <- rest;
         self;
      }
   };

};

class Main inherits IO {
   
   print_list(l : List) : Object {
      if l.isNil() then out_string("")
                   else {
			   out_string(l.head());
			   out_string("\n");
			   print_list(l.tail());
		        }
      fi
   };
   stack:List;
   str:String;
   str1:String;
   str2:String;
   i:Int;
   j:Int;
   k:Int;
   flag:Bool<-true;
 
   main() : Object {
      {
      stack<-new List;
      while flag loop
      {
         out_string(">");
         str<-in_string();
         
         -- out_string(str);
         -- out_string("\n");
         if str.length()=1 then {
            if str = "+" then
               stack<-stack.newNode(str)
            else if str="s" then
               stack<-stack.newNode(str)
            else if str="e" then
            {
               if (not stack.isNil()) then
               {
                  str<-stack.head();
               
                  if str="+" then{
                     stack<-stack.tail();
                     
                     if (not stack.isNil()) then 
                     {
                        str1<-stack.head();
                        stack<-stack.tail();
                        if (not stack.isNil()) then
                        {
                           str2<-stack.head();
                           stack<-stack.tail();
                           i<-(new A2I).c2i(str1);
                           j<-(new A2I).c2i(str2);
                           k<-i+j;
                           str<-(new A2I).i2c(k);
                           stack<-stack.newNode(str);
                        }else stack<-stack.newNode(str1) fi;
                     }else stack fi;
                  }
                  else if str="s" then{
                     stack<-stack.tail();
                     if (not stack.isNil()) then {
                        str1<-stack.head();
                        stack<-stack.tail();
                        if (not stack.isNil()) then{
                           str2<-stack.head();
                           stack<-stack.tail();
                           stack<-stack.newNode(str1);
                           stack<-stack.newNode(str2);
                        }else stack<-stack.newNode(str1) fi; 
                     }else stack fi;
                  }
                  else if str="" then stack
                  else{
                     -- stack<-stack.newNode(str);
                     stack;
                  }
                  fi fi fi;
               }else str fi;
            }
            else if str="d" then
               print_list(stack)
            else if str="x" then
               flag<-false
            else{
               stack<-stack.newNode(str);
            }
            fi fi fi fi fi;
         }
         else flag<-false fi;
      }
      pool;
   }

   };

};
