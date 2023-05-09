
(*
 *  execute "coolc bad.cl" to see the error messages that the coolc parser
 *  generates
 *
 *  execute "myparser bad.cl" to see the error messages that your parser
 *  generates
 *)

class A {
    -- feature error
    a:Int 
    true;
    c:Int;
    b:Int<-;
    assoc():Int{
        {
            a<=b<=2;
            ~a~a;
            NOT a NOT a;
        }
    };

    c(x:Int):{}; -- no return
    dispatch_error():Int{
        {
            a<-A;
            (a+b)@.print();
            a@A.print(2,);
            b@B.C(x);
            .print();
            A.print();
        }
    };
    formal_error(a,b,c){};
    statement_error():Int{
        {
            if
            -- then
            else
            fi;

            while();

            case 123
            -- of
            esac;

            1+-2;
            1+;

        }
        
    };
    
};

(* error:  b is not a type identifier *)
Class b inherits A {
};  

(* error:  a is not a type identifier *)
Class C inherits a {
};

(* error:  keyword inherits is misspelled *)
Class D inherts A {
};

(* error:  closing brace is missing *)
Class E inherits A {
;

