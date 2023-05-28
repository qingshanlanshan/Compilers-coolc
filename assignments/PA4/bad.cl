class C {
	a : Int;
	b : Bool;
	b : Int;
	self:Int;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
           }
	};
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
           }
	};
};

Class Main {
	main():C {
	 {
	  (new C).init(1,1);
	  (new C).init(1,true,3);
	  (new C).iinit(1,true);
	  (new C);
	  (new E);
	 }
	};
};

class A inherits C{
	b:Int;
	c:E;
	init(x:D,y:SELF_TYPE,x:Int,self:Int) : C {
		{
			b@E.print();
			b@C.print();
			c.print();
			self<-1;
			b;
			w<-1;
			if x.length() then
				(new C)<(new SELF_TYPE)
			else
				abc<=new A
			fi;
		}
	};
	init(x:Int,y:Int) : C {
		{
			while (new C) loop
				self
			pool;
			case (new C) of
			self : SELF_TYPE => c<-1; 
			x : Int => x<-1;
			x : Int => x<-1;
			x : E => self;
			esac;
			self;
			let self:E, y:Bool<-1 in z<-1;
			1+true;
			true-1;
			1*true;
			true/2;
			~true;
			true<false;
			true<=false;
			true=1;
			not 1;
		}
	};
};
class X inherits Int{};
-- class X inherits Y{};
-- class Y inherits X{};