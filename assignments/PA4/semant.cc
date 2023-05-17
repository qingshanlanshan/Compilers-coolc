

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <set>

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg = idtable.add_string("arg");
    arg2 = idtable.add_string("arg2");
    Bool = idtable.add_string("Bool");
    concat = idtable.add_string("concat");
    cool_abort = idtable.add_string("abort");
    copy = idtable.add_string("copy");
    Int = idtable.add_string("Int");
    in_int = idtable.add_string("in_int");
    in_string = idtable.add_string("in_string");
    IO = idtable.add_string("IO");
    length = idtable.add_string("length");
    Main = idtable.add_string("Main");
    main_meth = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any
    //   user-defined class.
    No_class = idtable.add_string("_no_class");
    No_type = idtable.add_string("_no_type");
    Object = idtable.add_string("Object");
    out_int = idtable.add_string("out_int");
    out_string = idtable.add_string("out_string");
    prim_slot = idtable.add_string("_prim_slot");
    self = idtable.add_string("self");
    SELF_TYPE = idtable.add_string("SELF_TYPE");
    Str = idtable.add_string("String");
    str_field = idtable.add_string("_str_field");
    substr = idtable.add_string("substr");
    type_name = idtable.add_string("type_name");
    val = idtable.add_string("_val");
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr)
{
    bool has_main_class = false, redefinition = false;
    /* Fill this in */
    install_basic_classes();
    for (int i = 0; classes->more(i); i++)
    {
        Class_ ith_class = classes->nth(i);
        // One main class requied
        if (ith_class->get_name() == Main)
        {
            has_main_class = true;
        }
        //
        if (class_table.find(ith_class->get_name()) != class_table.end() || ith_class->get_name() == IO || ith_class->get_name() == Int || ith_class->get_name() == Str || ith_class->get_name() == Bool || ith_class->get_name() == Object || ith_class->get_name() == SELF_TYPE)
        {
            semant_error(ith_class) << "Class " << ith_class->get_name() << " was previously defined." << std::endl;
            redefinition = true;
        }
        else
        {
            class_table.insert(std::pair<Symbol, Class_>(ith_class->get_name(), ith_class));
        }
    }
    if (has_main_class && !redefinition && !(invalid_parent = check_valid_parents()) && (inheritance_cycles = check_inherit_cycle()))
    {
        semant_error() << "Class Main is not defined." << std::endl;
    }
    object_table = new SymbolTable<Symbol, Symbol>();
    method_table = new SymbolTable<Symbol, Symbol>();
}

void ClassTable::check_feature()
{
    // cerr<<invalid_parent<<endl;
    if (inheritance_cycles||invalid_parent)
        return;
    for (auto it = class_table.begin(); it != class_table.end(); it++)
    {
        // for each non-basic class in the table
        if (it->second->get_name() == IO || it->second->get_name() == Int || it->second->get_name() == Str || it->second->get_name() == Bool || it->second->get_name() == Object)
            continue;
        auto features = it->second->get_features();
        object_table->enterscope();
        method_table->enterscope();
        Class_ cls = it->second;

        // for each feature of the current class
        // add feature to table
        for (int i = 0; features->more(i); i++)
        {
            Feature feature = features->nth(i);
            // method_class will call method_class::add_feature()
            // attr_class will call atte_class::add_feature()
            feature->add_feature(cls);
        }
        // main method required
        if (cls->get_name() == Main && method_table->probe(main_meth) == NULL)
            semant_error(cls) << "No 'main' method in class Main." << endl;

        // all features loaded, add formals of methods to object table
        // a complete feature table is required for checking exprs
        for (int i = features->first(); features->more(i); i = features->next(i))
        {
            Feature feature = features->nth(i);
            // method_class: check formals and expressions
            // attr_class: check redeclaration of parent attrs
            feature->update(cls);
        }

        object_table->exitscope();
        method_table->exitscope();
    }
}

void attr_class::update(Class_ cls)
{
    Symbol expr_type = init->check(cls);
    if (expr_type == SELF_TYPE)
        expr_type = cls->get_name();
    if (type_decl != SELF_TYPE && classtable->class_table.find(type_decl) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Class " << type_decl << " of attribute " << name << " is undefined." << endl;
    // check if the type of init expr matches type_decl
    if(expr_type!=No_type && !type_check(expr_type, type_decl))
     	classtable->semant_error(cls->get_filename(),this)<<"Inferred type "<<expr_type<<" of initialization of attribute "<<name<<" does not conform to declared type "<<type_decl<<"."<<endl;
}

// if child type is a parent type
bool type_check(Symbol child, Symbol parent)
{
    while (child != No_class)
    {
        if (child == parent)
            return true;
        child = classtable->class_table[child]->get_parent();
    }
    return false;
}

// get a method from a class and its parents
method_class *find_method(Symbol class_name, Symbol method_name)
{
    while (class_name != No_class)
    {
        auto it = classtable->class_table.find(class_name);
        if (it == classtable->class_table.end())
            break;

        Class_ cls = it->second;
        Features features = cls->get_features();
        // iterate over features to find the method
        for (int i = 0; features->more(i); i++)
        {
            Feature ith_feature = features->nth(i);
            if (!ith_feature->feature_type && ith_feature->get_name() == method_name)
                return (method_class *)ith_feature;
        }
        class_name = cls->get_parent();
    }
    return NULL;
}

attr_class *get_attr(Symbol class_name, Symbol attr_name)
{
    while (class_name != No_class)
    {
        // get the Class_ object
        Class_ cur_class = classtable->class_table[class_name];

        // get its features
        Features features = cur_class->get_features();

        // iterate over features to find the attr
        for (int i = 0; features->more(i); i++)
        {
            Feature ith_feature = features->nth(i);
            // if (ith_feature->feature_type && ith_feature->get_name() == attr_name)
            if (ith_feature->get_name() == attr_name)
                return (attr_class *)ith_feature;
        }
        class_name = cur_class->get_parent();
    }
    return NULL;
}

void method_class::update(Class_ cls)
{
    // enter a new scope and add formals to the object table
    classtable->object_table->enterscope();
    // check main method formals
    if (cls->get_name() == Main && name == main_meth && formals->len() != 0)
        classtable->semant_error(cls->get_filename(), this) << "'main' method in class Main should have no arguments." << endl;
    for (int i = 0; formals->more(i); i++)
        formals->nth(i)->add_formal(cls);

    // check the expr of the method body
    Symbol real_return_type = expr->check(cls);
    // classtable->semant_error(cls->get_filename(), this)<<"real return type:"<<real_return_type<<" return type:"<<return_type<<endl;
    // the actual return type exists (avoid type_check error)
    if (classtable->class_table.find(real_return_type) != classtable->class_table.end())
    {
        // the actual return type does not match the declared return type
        // cerr<<type_check(real_return_type, return_type)<<endl;
        if (!type_check(real_return_type, return_type))
            classtable->semant_error(cls->get_filename(), this) << "Inferred return type " << real_return_type<< " of method "<<name << " does not conform to declared return type " << return_type << "." << endl;
    }
    classtable->object_table->exitscope();
}

void formal_class::add_formal(Class_ cls)
{
    // check formal type
    // SELF_TYPE is not a valid type
    if (type_decl == SELF_TYPE)
        classtable->semant_error(cls->get_filename(), this) << "Formal parameter " << name << " cannot have type SELF_TYPE." << endl;
    // type is undefined
    else if (classtable->class_table.find(type_decl) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Class " << type_decl << " of formal parameter " << name << " is undefined." << endl;
    // self is not a valid name
    if (name == self)
        classtable->semant_error(cls->get_filename(), this) << "'self' cannot be the name of a formal parameter." << endl;
    // parameter name already used
    else if (classtable->object_table->probe(name) != NULL)
        classtable->semant_error(cls->get_filename(), this) << "Formal parameter " << name << " is multiply defined." << endl;
    // add to object table
    else
        classtable->object_table->addid(name, &type_decl);
}

void method_class::add_feature(Class_ cls)
{
    // *this is a method_class
    bool add_to_table = true;
    // return type undefined error
    if (return_type != SELF_TYPE && classtable->class_table.find(return_type) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Undefined return type " << return_type << " in method " << name << "." << endl;
    // method redefined error
    if (classtable->method_table->probe(name) != NULL)
    {
        classtable->semant_error(cls->get_filename(), this) << "Method " << name << " is multiply defined." << endl;
        add_to_table = false;
    }
    // to find the parent method
    // we iterate over its parent classes
    auto parent_method = find_method(cls->get_parent(), name);
    // if the parent method is found
    if (parent_method != NULL)
    {
        auto parent_formals = parent_method->get_formals();
        // different length difference error
        if (parent_method->get_formals()->len() != formals->len())
        {
            classtable->semant_error(cls->get_filename(), this) << "Incompatible number of formal parameters in redefined method " << name << "." << endl;
            add_to_table = false;
        }
        else
        {
            // same length, but different parameter type
            // for each parameter of the formals
            for (auto i = 0; formals->more(i); ++i)
                if (formals->nth(i)->get_type() != parent_formals->nth(i)->get_type())
                {
                    classtable->semant_error(cls->get_filename(), this) << "In redefined method " << name << ", parameter type " << formals->nth(i)->get_type() << " is different from original type " << parent_formals->nth(i)->get_type() << endl;
                    add_to_table = false;
                }
        }
    }
    if (add_to_table)
        classtable->method_table->addid(name, &return_type);
}

void attr_class::add_feature(Class_ cls)
{
    // *this is an attr_class
    bool add_to_table = true;
    // undefined type error
    // if (type_decl != SELF_TYPE && classtable->class_table.find(type_decl) == classtable->class_table.end())
    //     classtable->semant_error(cls->get_filename(), this) << "Class " << type_decl << " of attribute " << name << " is undefined." << endl;
    // multiply defined error
    if (classtable->object_table->probe(name) != NULL)
    {
        classtable->semant_error(cls->get_filename(), this) << "Attribute " << name << " is multiply defined in class." << endl;
        add_to_table = false;
    }
    // redefinition of 'self'
    if (name == self)
    {
        classtable->semant_error(cls->get_filename(), this) << "'self' cannot be the name of an attribute." << endl;
        add_to_table = false;
    }
    // check if attribute is pre-defined by parent classes
    attr_class *parent_attr = get_attr(cls->get_parent(), name);
    if (parent_attr != NULL)
        classtable->semant_error(cls->get_filename(), this) << "Attribute " << name << " is an attribute of an inherited class." << endl;
    else if (add_to_table)
        classtable->object_table->addid(name, &type_decl);
}

bool ClassTable::check_inherit_cycle()
{
    bool ret = false;
    for (auto it = class_table.begin(); it != class_table.end(); it++)
    {
        Symbol slow_pointer = it->first, fast_pointer = it->first;
        while (slow_pointer != Object && fast_pointer != Object && class_table[fast_pointer]->get_parent() != Object)
        {
            slow_pointer = class_table[slow_pointer]->get_parent();
            fast_pointer = class_table[fast_pointer]->get_parent();
            fast_pointer = class_table[fast_pointer]->get_parent();

            if (slow_pointer == fast_pointer)
            {
                semant_error(class_table[it->first]) << "Class " << it->first << ", or an ancestor of " << it->first << ", is involved in an inheritance cycle." << endl;
                ret = true;
                break;
            }
        }
    }
    return ret;
}

// return true if invalid parent is found, else false
bool ClassTable::check_valid_parents()
{
    bool ret = false;
    for (auto it = class_table.begin(); it != class_table.end(); it++)
    {
        if (it->first == Object)
            continue;
        auto parent = it->second->get_parent();
        if (class_table.find(parent) == class_table.end())
        {
            semant_error(it->second) << "Class " << it->first << " inherits from an undefined class " << parent << "." << endl;
            ret = true;
        }
        if (parent == SELF_TYPE || parent == Int || parent == Bool || parent == Str)
        {
            semant_error(it->second) << "Class " << it->first << " cannot inherit class " << parent << "." << endl;
            ret = true;
        }
    }
    return ret;
}

void ClassTable::install_basic_classes()
{
    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");

    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.

    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    //
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
        class_(Object,
               No_class,
               append_Features(
                   append_Features(
                       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                   single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
               filename);

    //
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class =
        class_(IO,
               Object,
               append_Features(
                   append_Features(
                       append_Features(
                           single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                                  SELF_TYPE, no_expr())),
                           single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                                  SELF_TYPE, no_expr()))),
                       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                   single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
               filename);

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer.
    //
    Class_ Int_class =
        class_(Int,
               Object,
               single_Features(attr(val, prim_slot, no_expr())),
               filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
        class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //
    Class_ Str_class =
        class_(Str,
               Object,
               append_Features(
                   append_Features(
                       append_Features(
                           append_Features(
                               single_Features(attr(val, Int, no_expr())),
                               single_Features(attr(str_field, prim_slot, no_expr()))),
                           single_Features(method(length, nil_Formals(), Int, no_expr()))),
                       single_Features(method(concat,
                                              single_Formals(formal(arg, Str)),
                                              Str,
                                              no_expr()))),
                   single_Features(method(substr,
                                          append_Formals(single_Formals(formal(arg, Int)),
                                                         single_Formals(formal(arg2, Int))),
                                          Str,
                                          no_expr()))),
               filename);
    class_table.insert(std::pair<Symbol, Class_>(Object, Object_class));
    class_table.insert(std::pair<Symbol, Class_>(IO, IO_class));
    class_table.insert(std::pair<Symbol, Class_>(Int, Int_class));
    class_table.insert(std::pair<Symbol, Class_>(Bool, Bool_class));
    class_table.insert(std::pair<Symbol, Class_>(Str, Str_class));
}

Symbol find_common_ancestor(Symbol type1, Symbol type2)
{

    if (type1 == No_type)
        return type2;
    if (type2 == No_type)
        return type1;

    int count1, count2;
    count1 = count2 = 0;
    Symbol temp;

    // count first
    for (temp = type1; temp != Object; temp = classtable->class_table[temp]->get_parent(), count1++)
        ;
    for (temp = type2; temp != Object; temp = classtable->class_table[temp]->get_parent(), count2++)
        ;

    while (count1 > count2)
    {
        type1 = classtable->class_table[type1]->get_parent();
        count1--;
    }

    while (count2 > count1)
    {
        type2 = classtable->class_table[type2]->get_parent();
        count2--;
    }

    while (type1 != type2)
    {
        type1 = classtable->class_table[type1]->get_parent();
        type2 = classtable->class_table[type2]->get_parent();
    }

    return type1;
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream &ClassTable::semant_error(Class_ c)
{
    return semant_error(c->get_filename(), c);
}

ostream &ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream &ClassTable::semant_error()
{
    semant_errors++;
    return error_stream;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    /* some semantic analysis code may go here */
    classtable->check_feature();

    if (classtable->errors())
    {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}

// Expression_class.check(Class_)
// name <- expr
Symbol assign_class::check(Class_ cls)
{
    // get the return type of the expr
    Symbol type = expr->check(cls);
    if (type == SELF_TYPE)
        type = cls->get_name();
    // get the type of the object
    Symbol *declared_type = classtable->object_table->lookup(name);
    // self cannot be assigned to
    if (name == self)
        classtable->semant_error(cls->get_filename(), this) << "Cannot assign to 'self'." << endl;
    // if the type is undeclared
    // it should be an attr declared earlier
    // set type of the object to the type of the attr
    else if (!declared_type)
    {
        // find the attr in current class and its parent classes
        auto attr = get_attr(cls->get_name(), name);
        if (attr)
        {
            auto temp = attr->get_type();
            declared_type = &temp;
        }
        else
            classtable->semant_error(cls->get_filename(), this) << "Assignment to undeclared variable " << name << "." << endl;
    }
    // the expr should has a declared type
    if (declared_type && !type_check(type, *declared_type))
        classtable->semant_error(cls->get_filename(), this) << "Type " << type << " of assigned expression does not conform to declared type " << *declared_type << " of identifier " << name << "." << endl;
    // the type of an assignment is the type of the expr
    set_type(type);
    return type;
}
// expr @ type_name . name ( actual )
Symbol static_dispatch_class::check(Class_ cls)
{
    Symbol t_expr = expr->check(cls);

    for (int i = 0; actual->more(i); i++)
        actual->nth(i)->check(cls);

    // if type_name does not exists
    if (classtable->class_table.find(type_name) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Static dispatch to undefined class " << type_name << "." << endl;
    // t_expr does not exist
    // or t_expr is not a subclass of type_name
    else if ((t_expr == SELF_TYPE || classtable->class_table.find(t_expr) == classtable->class_table.end()) && !type_check(t_expr, type_name))
        classtable->semant_error(cls->get_filename(), this) << "Expression type " << t_expr << " does not conform to declared static dispatch type " << type_name << "." << endl;
    // type check is good
    // check method
    else
    {
        auto method = find_method(type_name, name);
        // method does not exist
        if (method == NULL)
            classtable->semant_error(cls->get_filename(), this) << "Static dispatch to undefined method " << name << "." << endl;
        // length of actuals does not match length of formals
        else if (actual->len() != method->get_formals()->len())
            classtable->semant_error(cls->get_filename(), this) << "Method " << name << " called with wrong number of arguments." << endl;
        // check type of actuals
        else
        {
            for (int i = 0; i < actual->len(); i++)
            {
                auto type_actual = actual->nth(i)->get_type();
                auto type_formal = method->get_formals()->nth(i)->get_type();
                if (!type_check(type_actual, type_formal))
                    classtable->semant_error(cls->get_filename(), this) << "In call of method " << name << ", type " << type_actual << " of parameter " << method->get_formals()->nth(i)->get_name() << " does not conform to declared type " << type_formal << "." << endl;
            }
        }
        // type = method return type
        if (method != NULL)
        {
            Symbol t_method = method->get_type();
            if (t_method == SELF_TYPE)
                set_type(type_name);
            else
                set_type(t_method);
            return get_type();
        }
    }
    set_type(Object);
    return get_type();
}
// expr . name ( actual )
Symbol dispatch_class::check(Class_ cls)
{
    Symbol t_expr = expr->check(cls);
    // cerr<<"actual_len="<<actual->len()<<endl;
    // actual->dump(cerr,2);
    for (int i = actual->first(); actual->more(i); i = actual->next(i))
        actual->nth(i)->check(cls);
    if (t_expr != SELF_TYPE && classtable->class_table.find(t_expr) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Dispatch on undefined class " << t_expr << "." << endl;
    else
    {
        if (t_expr == SELF_TYPE)
            t_expr = cls->get_name();
        auto method = find_method(t_expr, name);
        if (!method)
            classtable->semant_error(cls->get_filename(), this) << "Dispatch to undefined method " << name << "." << endl;
        else
        {
            auto formals = method->get_formals();
            if (formals->len() != actual->len())
                classtable->semant_error(cls->get_filename(), this) << "Method " << name << " called with wrong number of arguments." << endl;
            else
            {
                for (int i = 0; i < formals->len(); i++)
                {
                    auto t_formal = formals->nth(i)->get_type();
                    auto t_actual = actual->nth(i)->get_type();
                    if (t_actual == SELF_TYPE)
                        t_actual = cls->get_name();
                    if (t_actual != t_formal)
                        classtable->semant_error(cls->get_filename(), this) << "In call of method " << name << ", type " << t_actual << " of parameter " << formals->nth(i)->get_name() << " does not conform to declared type " << t_formal << "." << endl;
                }
                set_type(method->get_type() != SELF_TYPE ? method->get_type() : t_expr);
                return get_type();
            }
        }
    }

    set_type(Object);
    return get_type();
}
// if pred then then_exp else else_exp
Symbol cond_class::check(Class_ cls)
{
    Symbol t_pred = pred->check(cls);
    Symbol t_then_exp = then_exp->check(cls);
    Symbol t_else_exp = else_exp->check(cls);

    if (t_pred != Bool)
        classtable->semant_error(cls->get_filename(), this) << "Predicate of 'if' does not have type Bool." << endl;
    if (t_then_exp == SELF_TYPE)
        t_then_exp = cls->get_name();
    if (t_else_exp == SELF_TYPE)
        t_else_exp = cls->get_name();
    auto ancestor = find_common_ancestor(t_then_exp, t_else_exp);
    set_type(ancestor);
    return get_type();
}
// while pred loop body pool
Symbol loop_class::check(Class_ cls)
{
    Symbol t_pred = pred->check(cls);
    body->check(cls);
    if (t_pred != Bool)
        classtable->semant_error(cls->get_filename(), cls) << "Loop condition does not have type Bool." << endl;
    return set_type(Object)->type;
}
// case expr of cases
// name : type_decl => expr
Symbol typcase_class::check(Class_ cls)
{
    Symbol t_expr = expr->check(cls);
    Symbol ret = No_type;
    std::set<Symbol> type_table;
    for (int i = 0; cases->more(i); i++)
    {
        Case ith_case = cases->nth(i);
        Symbol case_name = ith_case->get_name();
        Symbol case_type = ith_case->get_type();
        Symbol case_expr_type = ith_case->get_expr()->check(cls);

        classtable->object_table->enterscope();

        if (case_name == self)
            classtable->semant_error(cls->get_filename(), ith_case) << "'self' bound in 'case'." << endl;
        
        if (type_table.find(case_type) == type_table.end())
            type_table.insert(case_type);
        else
            classtable->semant_error(cls->get_filename(), this) << "Duplicate branch Int in case statement." << endl;
        if (classtable->class_table.find(case_type) == classtable->class_table.end())
            classtable->semant_error(cls->get_filename(), ith_case) << "Class " << case_type << " of case branch is undefined." << endl;
        if (case_expr_type == SELF_TYPE)
            case_expr_type = cls->get_name();
        ret = find_common_ancestor(ret, case_expr_type);
        classtable->object_table->exitscope();
    }
    return set_type(ret)->type;
}
Symbol block_class::check(Class_ cls)
{
    Symbol type_expr;
    // for each expr in block
    for (int i = 0; body->more(i); i++)
        type_expr = body->nth(i)->check(cls);

    set_type(type_expr);
    // classtable->semant_error(cls->get_filename(),this)<<"block type:"<<type_expr<<endl;
    return get_type();
}
// let identifier : type_decl <- init in body
Symbol let_class::check(Class_ cls)
{
    Symbol t_init = init->check(cls);
    
    if (type_decl != SELF_TYPE && classtable->class_table.find(type_decl) == classtable->class_table.end())
        classtable->semant_error(cls->get_filename(), this) << "Class " << type_decl << " of let-bound identifier " << identifier << " is undefined." << endl;
    else if (!type_check(t_init, type_decl))
        classtable->semant_error(cls->get_filename(), this) << "Inferred type " << t_init << " of initialization of " << identifier << " does not conform to identifier's declared type " << type_decl << "." << endl;
    // classtable->object_table->enterscope();
    if (identifier == self)
        classtable->semant_error(cls->get_filename(), this) << "'self' cannot be bound in a 'let' expression." << endl;
    else
        classtable->object_table->addid(identifier, &type_decl);
    // classtable->object_table->exitscope();

    Symbol t_body = body->check(cls);
    return set_type(t_body)->type;
}
// e1 + e2
Symbol plus_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " + " << t2 << endl;

    return set_type(Int)->type;
}
Symbol sub_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " - " << t2 << endl;

    return set_type(Int)->type;
}
Symbol mul_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " * " << t2 << endl;

    return set_type(Int)->type;
}
Symbol divide_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " / " << t2 << endl;

    return set_type(Int)->type;
}
// ~ e1
Symbol neg_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    if (t1 != Int)
        classtable->semant_error(cls->get_filename(), this) << "Argument of '~' has type " << t1 << " instead of Int." << endl;

    return set_type(Int)->type;
}
// e1 < e2
Symbol lt_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " < " << t2 << endl;

    return set_type(Bool)->type;
}
Symbol eq_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != t2 && (t1 == Int || t1 == Bool || t1 == Str || t2 == Int || t2 == Str || t2 == Bool))
        classtable->semant_error(cls->get_filename(), this) << "Illegal comparison with a basic type." << endl;
    return set_type(Bool)->type;
}
Symbol leq_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    Symbol t2 = e2->check(cls);
    if (t1 != Int || t2 != Int)
        classtable->semant_error(cls->get_filename(), this) << "non-Int arguments: " << t1 << " <= " << t2 << endl;

    return set_type(Bool)->type;
}
// not e1
Symbol comp_class::check(Class_ cls)
{
    Symbol t1 = e1->check(cls);
    if (t1 != Bool)
        classtable->semant_error(cls->get_filename(), this) << "Argument of 'not' has type " << t1 << " instead of Bool." << endl;

    return set_type(Bool)->type;
    ;
}
Symbol int_const_class::check(Class_ cls)
{
    return set_type(Int)->type;
}
Symbol bool_const_class::check(Class_ cls)
{
    return set_type(Bool)->type;
}
Symbol string_const_class::check(Class_ cls)
{
    return set_type(Str)->type;
}
// new type_name
Symbol new__class::check(Class_ cls)
{
    if (type_name != SELF_TYPE && classtable->class_table.find(type_name) == classtable->class_table.end())
    {
        classtable->semant_error(cls->get_filename(), cls) << "'new' used with undefined class " << type_name << "." << endl;
        return set_type(Object)->type;
    }
    else
        return set_type(type_name)->type;
    // return set_type(Object)->type;
}
Symbol isvoid_class::check(Class_ cls)
{
    e1->check(cls);
    return set_type(Bool)->type;
}
Symbol no_expr_class::check(Class_ cls)
{
    return set_type(No_type)->type;
}
// name
Symbol object_class::check(Class_ cls)
{
    // find the obj in the parent classes
    Symbol t_parent;
    attr_class *attr = get_attr(cls->get_parent(), name);
    if (attr)
        t_parent = attr->get_type();
    Symbol *t_cur = classtable->object_table->lookup(name);
    if (name == self)
        return set_type(SELF_TYPE)->type;
    // if not found in current class
    else if (!t_cur)
    {
        // and not found in parents
        if (!attr || !t_parent)
        // undeclared error
        {
            classtable->semant_error(cls->get_filename(), this) << "Undeclared identifier " << name << "." << endl;
            return set_type(Object)->type;
        }
        t_cur = &t_parent;
    }
    // classtable->semant_error(cls->get_filename(), this)<<*t_cur<<endl;
    set_type(*t_cur);
    // classtable->semant_error(cls->get_filename(), this)<<"1"<<endl;
    return get_type();
}