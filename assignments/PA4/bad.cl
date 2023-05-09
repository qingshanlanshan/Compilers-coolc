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
	 }
	};
};

class A inherits C{
	b:Int;
	c:E;
	init(x:D,y:SELF_TYPE,x:Int,self:Int) : C {
		{
			self<-1;
			b;
			w<-1;
		}
	};
	init(x:Int,y:Int) : C {
		{
			self;
		}
	};
};