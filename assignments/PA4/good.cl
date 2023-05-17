class C {
	a : Int;
	b : Bool;
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
			(new C).init(1,true);
			(new C)@C.init(1,true);
		}
	};
};

class A inherits C{
	init(x:Int,y:Bool) : C {
		{
			x<-1;
			if true then
				(new C)
			else
				self
			fi;
			while true loop
				self
			pool;
			case (new C) of
			x : Int => x<-1;
			x : C => self;
			esac;
			let z:Int<-1 in z<-1;
			1+2;
			4-1;
			1*2;
			4/2;
			~1;
			1<1;
			1<=3;
			2=1;
			not true;
			self;
			let x : Int, value : Int <- x + 1 in
				let value : Int <- 1 in
					x + value; 
		}
	};
};