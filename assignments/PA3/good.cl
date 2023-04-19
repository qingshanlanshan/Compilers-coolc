-- class A {
-- ana(): Int {
-- (let x:Int <- 1 in 2)+3
-- };
-- };

-- Class BB__ inherits A {
-- };

-- class C {
--     -- feature / math
--     a:Int;
--     b:Int<-3+3;
--     -- block / formal 
--     set_a(x:Int,y:Int):SELF_TYPE{
--         {
--             a<-x;
--             self;
--         }
--     };

--     const_test():SELF_TYPE{
--         {
--             123;
--             true;
--             self;
--         }
--     };

--     let_test(): INT {
-- 		let x:Int, y:Int, z:Object<-new Z IN
-- 			x<z.foo(y)
-- 	};



-- };
class C {
	dispatch1(): INT {
		let x:Int, y:Object<-new Y in 
			print(y<-x@Object.value())
	};
};