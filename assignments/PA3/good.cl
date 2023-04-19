class A {
ana(): Int {
(let x:Int <- 1 in 2)+3
};
};

Class BB__ inherits A {
};

class C {
    -- feature / math
    a:Int;
    b:Int<-3+3*3;
    -- block / formal 
    set_a(x:Int,y:Int):SELF_TYPE{
        {
            a<-x;
            self;
        }
    };

    const_test():SELF_TYPE{
        {
            123;
            true;
            self;
        }
    };

    let_test(): Int {
		let x:Int, y:Int, z:Object<-new Z IN
			x<z.foo(y)
	};

	试试utf8():Int{};
};


class C {
	dispatch1(): INT {
		let x:Int, y:Object<-new Y in 
			print(y<-x@Object.value())
	};
	dispatch2(): INT {
		let x:Int, y:Object<-new Y in 
			print(y<-x.,"123")
	};
	
};

class D {
	expression_test(): Int {
		{
			if
				1+2=3
			then
				case 1+2 of
					abc:Int=>NOT 123;
					def:Bool=>~1*(2+3)/4<5;
				esac
			else
			while 2<1
				loop
					true
				pool
			fi;
			false;
		}
	};
};